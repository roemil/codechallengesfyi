use std::collections::HashMap;
use std::io::{Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc;
use std::sync::mpsc::{Receiver, Sender};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, SystemTime, Instant};

fn handle_client(stream: Arc<TcpStream>, send_channel: Sender<Messages>) {
    // TODO: Move this to server side - easier to handle rate limiting
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
        .send(Messages::ClientConnected(stream.clone(), name_prefix.clone()))
        .map_err(|err| {
            eprintln!("ERROR: Could not notify server with new client: {err}");
        });

    let _ = writeln!(stream.as_ref(), "Welcome {:?} to the chat!", name_prefix).map_err(|err| {
        eprintln!("Failed to send welcome message: {err}");
    });

    loop {
        let mut vec = [0; 128];
        let n = stream.as_ref().read(&mut vec).map_err(|err| {
            eprintln!("ERROR: Client could not read incoming message: {err}");
        });
        if n == Ok(0) {
            return;
        }
        println!("INFO: Read n bytes: {:?}", n.unwrap());
        let text: Vec<u8> = vec[0..n.unwrap()]
            .iter()
            .cloned()
            .filter(|x| *x >= 32)
            .collect();
        let _ = send_channel
            .send(Messages::DistributeMessage(text.to_vec(), stream.peer_addr().unwrap()))
            .map_err(|err| {
                eprintln!("ERROR: Could not distribute message: {err}");
            });
    }
}
enum Messages {
    ClientConnected(Arc<TcpStream>, String),
    DistributeMessage(Vec<u8>, SocketAddr),
}

#[derive(Clone, Debug)]
struct Client {
    name: String,
    last_message: SystemTime,
    is_banned: bool,
    strikes: u8,
    stream: Arc<TcpStream>
}

fn is_allowed_to_send_msg(map : &mut HashMap<SocketAddr, Client>, sender : &SocketAddr) -> bool {
    match map.get_mut(&sender){
        Some(client) => {
            if client.is_banned {
                return false;
            } else {
                let now = SystemTime::now();
                let threshold = Duration::from_secs(1);
                if now.duration_since(client.last_message).unwrap() < threshold {
                    match client.strikes {
                        0..=1 => {
                            client.strikes += 1;
                            return true;
                        },
                        2 => {
                            client.strikes += 1;
                            client.is_banned = true;
                            writeln!(client.stream.as_ref(), "You are banned. LOL!").unwrap();
                            return false;
                        }
                        3 => {
                            return false;
                        },
                        num => {
                            panic!("Number of strikes exceeds valid number. {num}");
                        }
                    }
                } else {
                    return true
                }
            }
        },
        None => {
            eprintln!("INFO: Client no longer present in server: {}", sender);
            return false;
        },
    }
}

fn distribute_to_all_clients(streams: &Vec<Arc<TcpStream>>, sender : SocketAddr, name : &str, msg: &Vec<u8>) {
    for stream in streams {
        if stream.peer_addr().unwrap() != sender {
            let _ = writeln!(
                stream.as_ref(),
                "{:?}: {:?}",
                name,
                String::from_utf8(msg.clone()).unwrap()
            )
            .map_err(|err| {
                eprintln!("ERROR: Could not send message to client: {err}");
            });
        }
    }   
}

fn server(receiver: Receiver<Messages>) {
    let mut streams: Vec<Arc<TcpStream>> = Vec::new();
    let mut map: HashMap<SocketAddr, Client> = HashMap::new();
    loop {
        match receiver.recv() {
            Ok(data) => match data {
                Messages::ClientConnected(stream, name) => {
                    println!("INFO: Server has been notified of a new stream {}", stream.peer_addr().unwrap());
                    // TODO: Rate limit connections from same peer
                    streams.insert(streams.len(), stream.clone());
                    let client = Client {
                        stream: stream.clone(),
                        name,
                        last_message: SystemTime::now(),
                        is_banned: false,
                        strikes: 0,
                    };
                    map.insert(stream.peer_addr().unwrap(), client);
                }
                Messages::DistributeMessage(msg, sender) => {
                    // TODO: Rate limit messages from same sender.
                    if is_allowed_to_send_msg(&mut map, &sender) {
                        match map.get(&sender){
                            Some(client) => distribute_to_all_clients(&streams, sender, &client.name, &msg),
                            None => {
                                eprintln!("INFO: Client no longer present in server: {}", sender);
                            },
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
