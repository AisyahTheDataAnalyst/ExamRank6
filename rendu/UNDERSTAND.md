## 🚀 Overall Understanding

### 1. Global Setup (The "Party" Inventory)

* **`struct pollfd fds[]`**: This is your list of guests (sockets) to watch. `poll` needs this list to know which guests are trying to speak (send messages).


* **`t_client client[]`**: This stores the "label" for each guest. Each guest gets a unique ID.


* **`fdCount`**: This keeps track of how many people are currently at the party.


* **Buffers (`sendBuffer`, `recvBuffer`)**: Think of these as "whiteboards." One is for writing messages to be sent out, and the other is for reading messages coming in.



---

### 2. `main` (Opening the Venue)

1. **Arguments**: It checks if you gave it a port number. If not, it complains and leaves.


2. **Socket setup**: It creates a "doorway" (socket) for people to enter.


3. **Binding**: It assigns the venue to a specific port on `127.0.0.1` so people know where to find it.


4. **Listen**: It sets the "door" to accept visitors.


5. **Poll Loop**: This is the heart of the program. It sits in a `while(1)` loop, constantly asking the system: *"Is anyone new trying to come in, or is anyone already inside trying to say something?"*.



---

### 3. `newConnection` (Welcoming a Guest)

* When a new person arrives at the door, the server calls `accept` to officially let them in.


* It assigns them a **new ID** (starting from 0).


* It tells the "whiteboard" (`sendBuffer`) to announce to everyone already at the party: *"server: client [id] just arrived"*.



---

### 4. `handleClient` & `processMsg` (The Conversation)

* **`handleClient`**: If an existing guest speaks, the server calls `recv` to catch their message. If `recv` returns 0, it knows the guest has left.


* **`processMsg`**: This is the "translation" unit.


* It reads the guest's message byte by byte.


* It waits until it finds a `\n` (newline) character.


* Once it sees `\n`, it knows it has a full, complete message. It then formats the message to look like `client [id]: [message]` and asks `boardcastMsg` to yell it to everyone else.


* After broadcasting, it clears the message space so it is ready for the next one.





---

### 5. `boardcastMsg` (The Announcement System)

* The server loops through the `fds` list.


* If a guest is not the one who sent the message and is still at the party, it uses `send` to deliver the formatted message to them.



---

### 6. `disconnection` (Saying Goodbye)

* When a guest leaves, the server announces to everyone else: *"server: client [id] just left"*.


* It closes the guest's door (`close`).


* It removes the guest from the `fds` list and shifts everyone else down in the array so there are no empty gaps in the list.



---

### Summary Table for your Mental Map

| Function | Job |
| --- | --- |
| **`main`** | Sets up the house, listens, and waits for action. |
| **`newConnection`** | Welcomes new guests and gives them an ID. |
| **`handleClient`** | Listens to what a specific guest is saying. |
| **`processMsg`** | Cuts messages at the `\n` and prepares the format. |
| **`boardcastMsg`** | Sends a message to every guest except the sender. |
| **`disconnection`** | Removes a guest who has left and cleans up. |

You are looking at a system that organizes incoming data, sorts it by ID, and keeps the conversation flowing. You understand this logic now—it is just keeping track of a list of people and checking that list every time something changes.
<br>
<br>
<br>
<br>
<br>

## 🕵️ Function Understanding

### ` sprintf `

### What's it for?
- A function used to format data and store it into a string (a character array), rather than printing it to the screen like `printf`
- `sprintf` is a very useful alternative to `printf` because it allows you to construct formatted strings before sending them with `write` or `send`.

---

### Syntax
```c
int sprintf(char *str, const char *format, ...);
```
* **`str`**: Pointer to the character array (buffer) where you want the resulting text to be stored.
* **`format`**: The template string. Contains the text you want and "placeholders" like `%d` or `%s`
* **`...`**: These are the variables that replace the placeholders in the format string.
* → **Crucial Rule**: You must ensure your destination buffer is large enough to hold the final string to avoid memory errors.

---

### Pros

* **Automatic**: You do **not** need to manually clear or empty the `sendBuffer` before calling `sprintf`. It's designed to replace the content of the buffer every time.

	* **Overwrites:** When you call `sprintf(sendBuffer, ... )`, it starts writing from the very beginning of the buffer (`sendBuffer[0]`) and keeps going until it hits the end of your new string.

	* **Safety**: It automatically adds a null terminator (`\0`) to the end of your string, making it safe for other functions to read. 
		- Other functions (like `strlen` or `send`) stop reading exactly where your new message ends, effectively ignoring whatever old data might still be sitting in the rest of the buffer.

* **Memory-Safe Solution**: By using a fixed-size global buffer (like `sendBuffer` and `recvBuffer`) instead of dynamic memory allocation for every message, you avoid a massive amount of complexity. This approach is so much cleaner and safer, esp for an exam.

	* **No Memory Leaks:** Because you aren't using `malloc` for these specific buffers, you don't have to worry about `free()`ing them at the right time.

	* **Performance:** You avoid the overhead of the system constantly finding and releasing memory blocks, making your server faster and more predictable.

	* **Simplicity:** You eliminate an entire class of common bugs, such as "double free," "use-after-free," or memory fragmentation.
<br>
<br>


