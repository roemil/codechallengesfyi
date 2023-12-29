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
        .cloned()
        .filter(|x| *x >= 32)
        .collect();
    let name_prefix = String::from_utf8(name_text.to_vec()).unwrap();
    return name_prefix;
}

fn handle_client(stream: Arc<TcpStream>, send_channel: Sender<Messages>) {
    // TODO: Move this to server side - easier to handle rate limiting
    println!("Client connected!");
    let name = get_name_from_client(&stream);

    let _ = send_channel
        .send(Messages::ClientConnected(stream.clone(), name.clone()))
        .map_err(|err| {
            eprintln!("ERROR: Could not notify server with new client: {err}");
        });

    let _ = writeln!(stream.as_ref(), "Welcome {:?} to the chat!", name).map_err(|err| {
        eprintln!("Failed to send welcome message: {err}");
    });

    loop {
        let mut vec = [0; 128];
        let n = stream.as_ref().read(&mut vec).map_err(|err| {
            eprintln!("ERROR: Client could not read incoming message: {err}");
        });
        if n == Ok(0) {
            let _ = send_channel
                .send(Messages::ClientDisconnected(stream.peer_addr().unwrap()))
                .map_err(|err| {
                    eprintln!("ERROR: Could not distribute disconnect message: {err}");
                });
            return;
        }
        println!("INFO: Read n bytes: {:?}", n.unwrap());
        let text: Vec<u8> = vec[0..n.unwrap()]
            .iter()
            .cloned()
            .filter(|x| *x >= 32)
            .collect();
        let _ = send_channel
            .send(Messages::DistributeMessage(
                text.to_vec(),
                stream.peer_addr().unwrap(),
            ))
            .map_err(|err| {
                eprintln!("ERROR: Could not distribute message: {err}");
            });
    }
}
enum Messages {
    ClientConnected(Arc<TcpStream>, String),
    DistributeMessage(Vec<u8>, SocketAddr),
    ClientDisconnected(SocketAddr),
}

#[derive(Clone, Debug)]
struct Client {
    name: String,
    last_message: SystemTime,
    strikes: u8,
    stream: Arc<TcpStream>,
}


struct Server {
    clients: HashMap<SocketAddr, Client>,
    ban_list: HashMap<SocketAddr, SystemTime>,
}

impl Server {
    fn distribute_to_all_clients(&self,
        sender: SocketAddr,
        name: &str,
        msg: &Vec<u8>,
    ) {
        for (addr, client) in &self.clients {
            let stream = &client.stream;
            if *addr != sender {
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

    // TODO: Refactor this function
fn is_allowed_to_send_msg(&mut self, sender: &SocketAddr) -> bool {
    let now = SystemTime::now();
    match self.ban_list.get(&sender) {
        Some(banned_at) => {
            let time_left = Duration::from_secs(10) - now.duration_since(*banned_at).unwrap();
            if time_left > Duration::from_secs(0)
            {
                println!(
                    "INFO: Client {} is banned.",
                    sender
                );
                match self.clients.get(sender)
                {
                    Some(client) => {
                        writeln!(client.stream.as_ref(), "You are still banned for {:?}", time_left.as_secs()).unwrap();
                    },
                    None => {
                        eprintln!("Client not present in map");
                    }
                }
                return false;
            } else {
                println!(
                    "INFO: Client {} is no longer banned.",
                    sender
                );
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
                            match client.strikes {
                                0..=1 => {
                                    println!("Client {sender} got striked");
                                    client.strikes += 1;
                                    return true;
                                }
                                2 => {
                                    client.strikes += 1;
                                    self.ban_list.insert(*sender, now);
                                    writeln!(client.stream.as_ref(), "You are banned. LOL!").unwrap();
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
                    },
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
                    Messages::ClientConnected(stream, name) => {
                        println!(
                            "INFO: Server has been notified of a new stream {}",
                            stream.peer_addr().unwrap()
                        );
                        let now = SystemTime::now();
                        match self.clients.get_mut(&stream.peer_addr().unwrap()) {
                            Some(client) => {
                                client.stream = stream.clone();
                                client.name = name;
                                client.last_message = now;
                            },
                            None => {
                                let client = Client {
                                    stream: stream.clone(),
                                    name,
                                    last_message: now,
                                    strikes: 0,
                                };
                                self.clients.insert(stream.peer_addr().unwrap(), client);
                            }
                        }
                        // TODO: Rate limit connections from same peer

                    }
                    Messages::DistributeMessage(msg, sender) => {
                        // TODO: Rate limit messages from same sender.
                        if self.is_allowed_to_send_msg(&sender) {
                            match self.clients.get(&sender) {
                                Some(client) => {
                                    self.distribute_to_all_clients(sender, &client.name, &msg)
                                }
                                None => {
                                    eprintln!("INFO: Client no longer present in server: {}", sender);
                                }
                            }
                        }
                    }
                    Messages::ClientDisconnected(sender) => match self.clients.get(&sender) {
                        Some(client) => {
                            println!("Client {} disconnected. ", sender);
                            self.clients.remove(&client.stream.peer_addr().unwrap());
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

    let server : Server = Server { clients: HashMap::new(), ban_list: HashMap::new() };
    thread::spawn(move || Server::start(server, receiver));

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
