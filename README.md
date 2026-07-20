# 🌳 B+ Tree Visualizer

A file-persisted **B+ Tree** implemented in C, exposed as a CGI-style program and served through a small **Flask** web server, with an interactive **HTML/JS** front end for inserting values, displaying the tree level-by-level, and clearing it — all without needing a database.

---

## ✨ Features

- 🌲 B+ Tree of order 3 (max 2 keys per node) with **insert**, **level-order display**, and **clear all** operations
- 💾 **Persistent storage** — the tree lives in a binary file (`btree.dat`) on disk, so it survives across requests and server restarts
- 🔗 Node splitting with parent propagation, including new-root creation when the root overflows
- 🌐 CGI-style C program that reads `REQUEST_METHOD` / `CONTENT_LENGTH` and POST body like a classic CGI script
- 🐍 Flask server that compiles/runs the C executable and bridges it to the browser over HTTP
- 🎨 A polished single-page HTML/JS UI to insert numbers, view the tree structure, and clear it, with live feedback
- 🪟 One-click Windows launcher (`run.bat`) that compiles the C code and starts the server

---

## 📁 Project Structure

```
bplus-tree-visualizer/
├── bplus_tree.c        # B+ Tree logic (CGI program): insert, display, clear, file persistence
├── index.html           # Front-end UI: form, output panel, fetch() calls to the backend
├── start_server.py       # Flask server: serves index.html, runs bplus_tree.exe as a subprocess
├── run.bat               # Windows script: compiles bplus_tree.c then launches start_server.py
├── btree.dat             # Auto-generated: binary file storing the tree's node data
└── README.md
```

---

## 🧠 How It Works

1. **Storage layer** (`bplus_tree.c`) — Instead of keeping the tree in memory, every node (`NodeData`) is written to and read from a binary file (`btree.dat`) at a fixed offset ("file position" acts like a pointer). A `TreeMeta` header at the start of the file tracks the root's position, the next free offset, and the node count.
2. **Tree operations**:
   - `insert(key)` — finds the correct leaf (`find_leaf`), inserts the key in sorted order, and splits the node (`split_node`) if it exceeds `MAX_KEYS` (2 for order 3), propagating splits up to the root and creating a new root if needed.
   - `display_tree()` — walks the tree level by level (`display_level`) and prints each node's keys, marking leaves `(L)` and internal nodes `(I)`.
   - `clear_all()` — deletes `btree.dat` and reinitializes an empty tree.
3. **CGI bridge** — `main()` in `bplus_tree.c` mimics a CGI program: it reads `operation` (`insert`/`display`/`clear`) and `value` from a URL-encoded POST body via `stdin`, using the `REQUEST_METHOD` and `CONTENT_LENGTH` environment variables, and prints a `Content-Type` header followed by plain-text output — exactly what a CGI-compliant web server expects.
4. **Flask server** (`start_server.py`) — Rather than running a real CGI server, it serves `index.html` at `/` and, on `POST /cgi-bin/bplus_tree.exe`, re-builds the same POST body, sets the same CGI environment variables, and runs `bplus_tree.exe` as a subprocess, forwarding its stdout back to the browser (stripping the CGI header first).
5. **Front end** (`index.html`) — A single page with an input box and three buttons (Insert / Display / Clear), using `fetch()` to POST to `/cgi-bin/bplus_tree.exe` and rendering the raw text response — which is the ASCII tree printed by the C program — inside the output panel.

---

## 🚀 Getting Started

### 1. Prerequisites
- **GCC** (or another C compiler) available on your PATH — e.g., via [MinGW-w64](https://www.mingw-w64.org/) on Windows, or `build-essential` on Linux/macOS
- **Python 3.9+**
- **Flask**: `pip install flask`

### 2. Configure the project path (important on Windows)
`start_server.py` currently hardcodes the working directory:
```python
BASE_DIR = r'd:\momen\Projects\Programming\C++\B+tree'
```
Update this to wherever you've placed the project files, or replace it with something portable like:
```python
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
```

### 3. Compile and run

**Windows (one command):**
```bat
run.bat
```
This compiles `bplus_tree.c` into `bplus_tree.exe` with GCC, then starts the Flask server.

**macOS/Linux (manual steps):**
```bash
gcc bplus_tree.c -o bplus_tree
pip install flask
python start_server.py
```
> Note: `start_server.py` invokes `bplus_tree.exe` by name — on macOS/Linux, rename the compiled binary to `bplus_tree.exe` or update the `subprocess.run([...])` call in `start_server.py` to match your compiled binary's name (e.g., `./bplus_tree`).

### 4. Open the visualizer
Once the server is running, open your browser to:
```
http://localhost:5000
```

---

## 🕹️ Using the Visualizer

- **Insert Value** — type a number and click **Insert Value** (or press Enter) to add it to the tree. Duplicate keys are rejected.
- **Display Tree** — shows the tree level by level, e.g. `[10,20](L)` for a leaf node containing keys 10 and 20, or `(I)` for internal nodes.
- **Clear All Nodes** — wipes `btree.dat` and resets to an empty tree (with a confirmation prompt).
- The tree automatically loads and displays its current state when the page first opens.

---

## 🛠️ Requirements

```
flask
```
Plus a working C compiler (GCC recommended) on your system PATH.

---

## 📌 Notes & Possible Improvements

- `BASE_DIR` in `start_server.py` is hardcoded to a specific machine path — swapping it for a relative/`__file__`-based path would make the project portable out of the box.
- The Flask app runs with `debug=False` and binds to `0.0.0.0:5000` — fine for local use, but review this before exposing it beyond localhost, since the CGI bridge executes a local binary based on client-controlled form data.
- `bplus_tree.c` currently only supports **insert**, **display**, and **clear all** — there's no key-deletion (removal of a single key) operation yet, which would be a natural extension of a full B+ Tree implementation.
- Consider adding a `requirements.txt` with `flask` pinned, and a cross-platform note/Makefile so the same `run` step works without editing `start_server.py` by hand on macOS/Linux.
