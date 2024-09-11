use std::alloc::System;
use std::collections::HashMap;
use std::io::{Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc;
use std::sync::mpsc::{Receiver, Sender};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, SystemTime};

/*
TODO:
First client chooses name - done
then the authenticate with token - here we can check if the client tries to connect to often - not done
Then we can rate limit messages from client - done
... something else?
UI for client perhaps

*/

fn get_name_from_client(stream: &Arc<TcpStream>) -> String {
    let _ = stream.as_ref().write(b"Enter your name: ").map_err(|err| {
        eprintln!("Failed to ask for name: {err}");
    });
    let mut name = [0; 128];
    let n = stream.as_ref().read(&mut name).map_err(|err| {
        eprintln!("ERROR: could not read name: {err}");
    });
    let name_text: Vec<u8> = name[0..n.unwrap()]
        .iter()
        .filter(|x| **x >= 32)
        .map(|x| *x)
        .collect();
    match String::from_utf8(name_text) {
        Ok(str) => str,
        Err(e) => {
            eprintln!("Failed to convert to str: {}", e);
            String::new()
        }
    }
}

fn handle_client(stream: Arc<TcpStream>, send_channel: Sender<Messages>) {
    println!("Client connected!");
    let name = get_name_from_client(&stream);

    let _ = writeln!(stream.as_ref(), "Welcome {:?} to the chat!", name).map_err(|err| {
        eprintln!("Failed to send welcome message: {err}");
    });
    let prefix: Vec<u8> = String::from(name.clone() + ": ")
        .as_bytes()
        .iter()
        .filter_map(|x| if *x >= 32 { Some(*x) } else { None })
        .collect();

    let addr = match stream.peer_addr() {
        Ok(addr) => addr,
        Err(_) => return,
    };

    loop {
        let mut vec = [0; 128];
        let n = stream.as_ref().read(&mut vec).map_err(|err| {
            eprintln!("ERROR: Client could not read incoming message: {err}");
        });
        if n == Ok(0) {
            let _ = send_channel
                .send(Messages::ClientDisconnected(addr))
                .map_err(|err| {
                    eprintln!("ERROR: Could not distribute disconnect message: {err}");
                });
            return;
        }
        println!("INFO: Read n bytes: {:?}", n.unwrap());
        let text: Vec<u8> = vec[0..n.unwrap()]
            .iter()
            .filter_map(|x| if *x >= 32 { Some(*x) } else { None })
            .collect();
        let _ = send_channel
            .send(Messages::DistributeMessage(
                [prefix.clone(), text].concat(),
                addr,
            ))
            .map_err(|err| {
                eprintln!("ERROR: Could not distribute message: {err}");
            });
    }
}

enum Messages {
    ClientConnected(Arc<TcpStream>),
    DistributeMessage(Vec<u8>, SocketAddr),
    ClientDisconnected(SocketAddr),
}

#[derive(Clone, Debug)]
struct Client {
    last_message: SystemTime,
    last_connection: SystemTime,
    msg_strikes: u8,
    conn_strikes: u8,
    stream: Arc<TcpStream>,
    is_connected: bool,
}

struct Server {
    clients: HashMap<SocketAddr, Client>,
    ban_list: HashMap<SocketAddr, SystemTime>,
}

fn is_ddos(now: &SystemTime, client: &Client) -> bool {
    let threshold = Duration::from_millis(5000);
    now.duration_since(client.last_connection)
        .expect("Cannot go back in time")
        < threshold
}

impl Server {
    fn distribute_to_all_clients(&mut self, sender: SocketAddr, msg: &Vec<u8>) {
        let mut closed_streams: Vec<SocketAddr> = Vec::new();
        for (addr, client) in &self.clients {
            let stream = &client.stream;
            if client.is_connected && *addr != sender {
                if let Err(e) = writeln!(
                    stream.as_ref(),
                    "{:?}",
                    String::from_utf8(msg.clone()).unwrap()
                ) {
                    eprintln!("ERROR: Could not send message to client: {e}");
                    closed_streams.push(*addr);
                }
            }
        }
        for addr in &mut closed_streams {
            if let Some(client) = self.clients.get_mut(addr) {
                client.is_connected = false;
            }
        }
    }

    // TODO: Refactor this function
    fn is_allowed_to_send_msg(&mut self, sender: &SocketAddr) -> bool {
        let now = SystemTime::now();
        match self.ban_list.get(&sender) {
            Some(banned_at) => {
                let time_left = Duration::from_secs(10)
                    - now
                        .duration_since(*banned_at)
                        .expect("Cant go back in time");
                if time_left > Duration::from_secs(0) {
                    println!("INFO: Client {} is banned.", sender);
                    match self.clients.get(sender) {
                        Some(client) => {
                            writeln!(
                                client.stream.as_ref(),
                                "You are still banned for {:?}",
                                time_left.as_secs()
                            )
                            .unwrap();
                        }
                        None => {
                            eprintln!("Client not present in chat");
                        }
                    }
                    return false;
                } else {
                    println!("INFO: Client {} is no longer banned.", sender);
                    self.ban_list.remove(sender);
                    return true;
                }
            }
            None => {
                let threshold = Duration::from_millis(500);
                match self.clients.get_mut(sender) {
                    Some(client) => {
                        if now.duration_since(client.last_message).unwrap() < threshold {
                            client.last_message = now;
                            match client.msg_strikes {
                                0..=1 => {
                                    println!("Client {sender} got striked");
                                    client.msg_strikes += 1;
                                    return true;
                                }
                                2 => {
                                    client.msg_strikes += 1;
                                    self.ban_list.insert(*sender, now);
                                    writeln!(client.stream.as_ref(), "You are banned. LOL!")
                                        .unwrap();
                                    return false;
                                }
                                num => {
                                    panic!("Number of strikes exceeds valid number. {num}");
                                }
                            }
                        } else {
                            client.last_message = now;
                            return true;
                        }
                    }
                    None => {
                        eprintln!("Client not present in map {sender}");
                        return false;
                    }
                }
            }
        }
    }

    fn start(mut self, receiver: Receiver<Messages>) {
        loop {
            match receiver.recv() {
                Ok(data) => match data {
                    Messages::ClientConnected(stream) => {
                        println!(
                            "INFO: Server has been notified of a new stream {}",
                            stream.peer_addr().unwrap()
                        );

                        let now = SystemTime::now();
                        match self.clients.get_mut(&stream.peer_addr().unwrap()) {
                            Some(client) => {
                                println!("Old client");
                                if is_ddos(&now, &client) {
                                    client.conn_strikes += 1;
                                }
                                if client.conn_strikes > 3 {
                                    self.ban_list.insert(stream.peer_addr().unwrap(), now);
                                    writeln!(stream.as_ref(), "You are banned. LOL!").unwrap();
                                    let _ = stream.shutdown(std::net::Shutdown::Both);
                                } else {
                                    client.stream = stream.clone();
                                    client.last_message = now;
                                    client.last_connection = now;
                                }
                            }
                            None => {
                                println!("New client");

                                let client = Client {
                                    stream: stream.clone(),
                                    last_message: now,
                                    last_connection: now,
                                    msg_strikes: 0,
                                    conn_strikes: 0,
                                    is_connected: true,
                                };
                                self.clients.insert(stream.peer_addr().unwrap(), client);
                            }
                        }
                        // TODO: Rate limit connections from same peer
                    }
                    Messages::DistributeMessage(msg, sender) => {
                        if self.is_allowed_to_send_msg(&sender) {
                            self.distribute_to_all_clients(sender, &msg);
                        }
                    }
                    Messages::ClientDisconnected(sender) => match self.clients.get_mut(&sender) {
                        Some(client) => {
                            println!("Client {} disconnected. ", sender);
                            client.is_connected = false;
                            //self.clients.remove(&client.stream.peer_addr().unwrap());
                        }
                        None => {
                            eprintln!("INFO: Client no longer present in server: {}", sender);
                        }
                    },
                },
                Err(e) => {
                    eprintln!("ERROR: Could not receive in server channel: {e}");
                }
            }
        }
    }
}

fn main() {
    let listener = TcpListener::bind("127.0.0.1:7007").map_err(|err| {
        eprintln!("ERROR: Cannot bind to localhost. Err: {err}");
    });

    let (sender, receiver): (Sender<Messages>, Receiver<Messages>) = mpsc::channel();

    let server: Server = Server {
        clients: HashMap::new(),
        ban_list: HashMap::new(),
    };
    thread::spawn(move || Server::start(server, receiver));

    match listener {
        Ok(listener) => {
            for stream in listener.incoming() {
                match stream {
                    Ok(stream) => {
                        let send_channel = sender.clone();
                        let tcp_stream = Arc::new(stream).clone();
                        let _ = send_channel.send(Messages::ClientConnected(tcp_stream.clone()));
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
