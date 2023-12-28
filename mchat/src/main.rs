use std::io::{Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc;
use std::sync::mpsc::{Receiver, Sender};
use std::sync::Arc;
use std::thread;

#[derive(Clone)]
struct Client {
    name: String,
    addr: SocketAddr,
}

fn handle_client(stream: Arc<TcpStream>, send_channel: Sender<Messages>) {
    println!("Client connected!");
    let _ = stream.as_ref().write(b"Enter your name: ").map_err(|err| {
        eprintln!("Failed to ask for name: {err}");
    });
    let mut name = [0; 128];
    let n = stream.as_ref().read(&mut name).map_err(|err| {
        eprintln!("ERROR: could not read name: {err}");
    });
    let name_text: Vec<u8> = name[0..n.unwrap()]
        .iter()
        .cloned()
        .filter(|x| *x >= 32)
        .collect();
    let name_prefix = String::from_utf8(name_text.to_vec()).unwrap();

    let _ = send_channel
        .send(Messages::ClientConnected(stream.clone()))
        .map_err(|err| {
            eprintln!("ERROR: Could not notify server with new client: {err}");
        });

    let _ = writeln!(stream.as_ref(), "Welcome {:?} to the chat!", name_prefix).map_err(|err| {
        eprintln!("Failed to send welcome message: {err}");
    });
    let client = Client {
        name: name_prefix.clone(),
        addr: stream.peer_addr().unwrap(),
    };
    loop {
        let mut vec = [0; 128];
        let n = stream.as_ref().read(&mut vec).map_err(|err| {
            eprintln!("ERROR: Client could not read incoming message: {err}");
        });
        println!("INFO: Read n bytes: {:?}", n);
        let text: Vec<u8> = vec[0..n.unwrap()]
            .iter()
            .cloned()
            .filter(|x| *x >= 32)
            .collect();
        let _ = send_channel
            .send(Messages::DistributeMessage(text.to_vec(), client.clone()))
            .map_err(|err| {
                eprintln!("ERROR: Could not distribute message: {err}");
            });
    }
}
enum Messages {
    ClientConnected(Arc<TcpStream>),
    DistributeMessage(Vec<u8>, Client),
}

fn server(receiver: Receiver<Messages>) {
    let mut streams: Vec<Arc<TcpStream>> = Vec::new();
    loop {
        match receiver.recv() {
            Ok(data) => match data {
                Messages::ClientConnected(stream) => {
                    println!("INFO: Got new stream");
                    // TODO: Rate limit connections from same peer
                    streams.insert(streams.len(), stream.clone());
                }
                Messages::DistributeMessage(msg, sender) => {
                    // TODO: Rate limit messages from same sender.
                    for stream in &streams {
                        if stream.peer_addr().unwrap() != sender.addr {
                            let _ = writeln!(
                                stream.as_ref(),
                                "{:?}: {:?}",
                                sender.name,
                                String::from_utf8(msg.clone()).unwrap()
                            )
                            .map_err(|err| {
                                eprintln!("ERROR: Could not send message to client: {err}");
                            });
                        }
                    }
                }
            },
            Err(e) => {
                eprintln!("ERROR: Could not receive in server channel: {e}");
            }
        }
    }
}

fn main() {
    let listener = TcpListener::bind("127.0.0.1:7007").map_err(|err| {
        eprintln!("ERROR: Cannot bind to localhost. Err: {err}");
    });

    let (sender, receiver): (Sender<Messages>, Receiver<Messages>) = mpsc::channel();

    thread::spawn(move || server(receiver));

    match listener {
        Ok(listener) => {
            // accept connections and process them concurrently
            for stream in listener.incoming() {
                match stream {
                    Ok(stream) => {
                        let send_channel = sender.clone();
                        let tcp_stream = Arc::new(stream).clone();
                        thread::spawn(move || handle_client(tcp_stream, send_channel));
                    }
                    Err(e) => {
                        eprintln!("ERROR: failed to accept stream: {e}");
                    }
                }
            }
        }
        Err(e) => {
            eprintln!("ERROR: failed to listen: {:?}", e);
        }
    }
}
