use rand::RngCore;
use std::collections::HashMap;
use std::error::Error;
use std::io::{Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc;
use std::sync::mpsc::{Receiver, Sender};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, SystemTime};

/*
TODO:
then the authenticate with token - here we can check if the client tries to connect to often - not done
... something else?
UI for client perhaps

*/

fn get_name_from_client(stream: &Arc<TcpStream>) -> Result<String, Box<dyn Error>> {
    stream.as_ref().write(b"Enter your name: ")?;
    let mut name = [0; 128];
    let n = stream.as_ref().read(&mut name)?;
    let name_text: Vec<u8> = name[0..n]
        .iter()
        .filter(|x| **x >= 32)
        .map(|x| *x)
        .collect();
    let name = String::from_utf8(name_text)?;
    println!("Got name: {}", name);
    Ok(name)
}

fn handle_client(stream: Arc<TcpStream>, send_channel: Sender<Messages>) {
    println!("Client connected!");

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
            .send(Messages::DistributeMessage(text, addr))
            .map_err(|err| {
                eprintln!("ERROR: Could not distribute message: {err}");
            });
    }
}

enum Messages {
    ClientConnected(Arc<TcpStream>, Sender<Messages>),
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
    name: String,
}

impl Client {
    pub fn new(stream: Arc<TcpStream>, last_seen: SystemTime, name: String) -> Self {
        Client {
            stream: stream,
            last_message: last_seen,
            last_connection: last_seen,
            msg_strikes: 0,
            conn_strikes: 0,
            is_connected: true,
            name,
        }
    }
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
                if let Ok(msg) = String::from_utf8(msg.clone()) {
                    if let Err(e) = writeln!(stream.as_ref(), "{:?}", msg) {
                        eprintln!("ERROR: Could not send message to client: {e}");
                        closed_streams.push(*addr);
                    }
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
                let time_passed = now
                    .duration_since(*banned_at)
                    .expect("Cant go back in time");
                let threshold = Duration::from_secs(10);
                if time_passed < threshold {
                    println!("INFO: Client {} is banned.", sender);
                    match self.clients.get(sender) {
                        Some(client) => {
                            writeln!(
                                client.stream.as_ref(),
                                "You are still banned for {:?}",
                                (threshold - time_passed).as_secs()
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
                    if let Some(client) = self.clients.get_mut(sender) {
                        client.msg_strikes = 0;
                    }
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

    fn start(mut self, receiver: Receiver<Messages>, token: String) -> Result<(), ()> {
        loop {
            let received_data = receiver
                .recv()
                .map_err(|e| eprintln!("ERROR: Could not receive in server channel: {e}"))?;
            self.purge_disconnected_clients();

            match received_data {
                Messages::ClientConnected(stream, send_ch) => {
                    println!(
                        "INFO: Server has been notified of a new stream {}",
                        stream.peer_addr().unwrap()
                    );
                    match authenticate(&stream, &token.as_bytes().to_vec()) {
                        Ok(is_authenticated) => {
                            if !is_authenticated {
                                let _ = stream.as_ref().write(b"Authentication failed");
                                println!("Authentication failed");
                                continue;
                            }
                            println!("Authenticated");
                        }
                        Err(_) => {
                            println!("Error while authenticating");
                            continue;
                        }
                    }

                    let name = match get_name_from_client(&stream) {
                        Ok(name) => name,
                        Err(e) => {
                            println!("Failed to get name: {}", e);
                            continue;
                        }
                    };

                    let now = SystemTime::now();
                    let addr = stream.peer_addr().unwrap();
                    match self.clients.get_mut(&addr) {
                        Some(client) => {
                            if is_ddos(&now, &client) {
                                client.conn_strikes += 1;
                            }
                            if client.conn_strikes > 3 {
                                self.ban_list.insert(stream.peer_addr().unwrap(), now);
                                let _ = writeln!(stream.as_ref(), "You are banned. LOL!")
                                    .map_err(|e| println!("failed to notify banned client: {}", e));
                                let _ = stream.shutdown(std::net::Shutdown::Both);
                                continue;
                            } else {
                                client.stream = stream.clone();
                                client.last_message = now;
                                client.last_connection = now;
                                client.is_connected = true;
                                client.name = name.clone();
                            }
                        }
                        None => {
                            self.clients.insert(
                                stream.peer_addr().unwrap(),
                                Client::new(stream.clone(), now, name.clone()),
                            );
                        }
                    }
                    if let Err(e) = writeln!(stream.as_ref(), "Welcome {:?} to the chat!", name) {
                        eprintln!("Failed to send welcome message: {e}");
                        self.mark_as_disconnected(&addr);
                        continue;
                    }
                    thread::spawn(move || handle_client(stream, send_ch));
                }
                Messages::DistributeMessage(msg, sender) => {
                    if self.is_allowed_to_send_msg(&sender) {
                        self.distribute_to_all_clients(sender, &msg);
                    }
                }
                Messages::ClientDisconnected(sender) => {
                    if let Some(client) = self.clients.get_mut(&sender) {
                        println!("Client {} disconnected. ", sender);
                        client.is_connected = false;
                    }
                }
            }
        }
    }
    fn purge_disconnected_clients(&mut self) {
        let now = SystemTime::now();
        let threshold = Duration::from_secs(20);
        self.clients.retain(|_, client| {
            client.is_connected || now.duration_since(client.last_message).unwrap() < threshold
        });
    }

    fn mark_as_disconnected(&mut self, addr: &SocketAddr) {
        if let Some(client) = self.clients.get_mut(addr) {
            println!("Client {} disconnected. ", addr);
            client.is_connected = false;
        }
    }
}

struct AuthenticationError;

fn authenticate(
    stream: &Arc<TcpStream>,
    expected_token: &Vec<u8>,
) -> Result<bool, AuthenticationError> {
    if let Err(e) = stream.as_ref().write(b"Token: ") {
        eprintln!("Failed to ask for token: {e}");
        return Err(AuthenticationError);
    }
    let mut token: Vec<u8> = vec![0; 17];
    let n = match stream.as_ref().read(&mut token) {
        Ok(n) => n,
        Err(e) => {
            eprintln!("Failed to read token answer: {e}");
            return Err(AuthenticationError);
        }
    };
    let token: Vec<u8> = token
        .iter()
        .filter_map(|x| if *x >= 32 { Some(*x) } else { None })
        .collect();
    let token = &token[0..16];
    println!("Read token bytes: {}", n);
    if n < expected_token.len() {
        println!(
            "Did not read enough bytes for token to be correct. Expected: {}, got: {}",
            expected_token.len(),
            n
        );
        return Ok(false);
    }
    println!("token: {:?}, expected: {:?}", token, expected_token);
    Ok(token == *expected_token)
}

fn generate_token() -> String {
    let mut bytes = [0; 8];
    rand::thread_rng().fill_bytes(&mut bytes);
    println!("{}", hex::encode(&bytes).to_ascii_uppercase());
    hex::encode(bytes).to_ascii_uppercase()
}

fn main() -> Result<(), ()> {
    let listener = TcpListener::bind("127.0.0.1:7007").map_err(|err| {
        eprintln!("ERROR: Cannot bind to localhost. Err: {err}");
    })?;

    let (sender, receiver): (Sender<Messages>, Receiver<Messages>) = mpsc::channel();

    let server: Server = Server {
        clients: HashMap::new(),
        ban_list: HashMap::new(),
    };
    let token = generate_token();
    thread::spawn(move || Server::start(server, receiver, token));

    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                let send_channel = sender.clone();
                let tcp_stream = Arc::new(stream).clone();
                let _ = send_channel.send(Messages::ClientConnected(
                    tcp_stream.clone(),
                    send_channel.clone(),
                ));
            }
            Err(e) => {
                eprintln!("ERROR: failed to accept stream: {e}");
            }
        }
    }
    Ok(())
}
