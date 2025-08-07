# Virtual File System with Linux-like DAC & Audit Logging

## 📌 Overview

This project implements a **Virtual File System (VFS)** in C with **Linux-style Discretionary Access Control (DAC)**, **user/group management**, and a **full audit logging system**.

It simulates core Linux commands such as `mkdir`, `touch`, `ls`, `cd`, `rm`, `chown`, and `chmod`, along with user/group administration commands like `useradd`, `groupadd`, `usermod`, and `deluser`.

Every important action is logged into an `audit.log` file with a timestamp, acting like a simplified `syslog` for the virtual system.

The data entered persists through `vfs.txt`, `users.txt` and `groups.txt`.

---

## ✨ Features

* **Virtual File System (VFS)** with:

  * File and directory creation/deletion
  * Navigation (`cd`, `pwd`)
  * Read/write operations
  * Permission handling (owner, group, others)
  * Persistent storage & loading
* **User & Group Management**

  * Add/remove users and groups
  * Modify group memberships (`usermod -a -G`)
* **Linux-like DAC Permissions**

  * Owner/Group/Others permission checks
  * Commands like `chmod` and `chown`
* **Audit Logging**

  * Tracks all user actions
  * Logs include: timestamp, user, action, target, status
  * Saved in `audit.log`

---

## 📂 Project Structure

```
project/
├── main.c                     # CLI command parser
├── audit/
│   └── audit.c / audit.h       # Audit trail
├── virtual-file-system/
│   └── vfs.c / vfs.h           # VFS implementation
├── user-group-management/
│   ├── user.c / user.h         # User handling
│   ├── group.c / group.h       # Group handling
│   └── usermod.c / usermod.h   # User-group linking
├── users.txt                   # Stored users & groups
├── vfs.txt                     # Persistent VFS storage
└── audit.log                   # Action logs
```

---

## 🖥️ Commands

### **User & Group Management**

| Command                        | Description        |
| ------------------------------ | ------------------ |
| `useradd <username>`           | Create a new user  |
| `groupadd <groupname>`         | Create a new group |
| `usermod -a -G <group> <user>` | Add user to group  |
| `deluser <username>`           | Delete a user      |
| `delgroup <groupname>`         | Delete a group     |

### **Login/Logout**

| Command            | Description          |
| ------------------ | -------------------- |
| `login <username>` | Log in as a user     |
| `logout`           | Log out current user |

### **File System**

| Command                         | Description                  |
| ------------------------------- | ---------------------------- |
| `mkdir <name>`                  | Create a directory           |
| `touch <name>`                  | Create a file                |
| `ls [-l]`                       | List files and directories   |
| `cd <path>`                     | Change directory             |
| `pwd`                           | Show current directory path  |
| `write <file> <content>`        | Write to file                |
| `read <file>`                   | Read file contents           |
| `rm <file>`                     | Delete file                  |
| `rm -r <dir>`                   | Delete directory recursively |
| `tree`                          | Show directory structure     |
| `chown <user>:<group> <target>` | Change owner/group           |
| `chmod <permissions> <target>`  | Change permissions           |
| `save`                          | Save VFS to `vfs.txt`        |
| `load`                          | Load VFS from `vfs.txt`      |
| `exit`                          | Save and exit                |

---

## 📜 Audit Logging

Every executed command generates a log entry:

```
[YYYY-MM-DD HH:MM:SS] USER='<username>' ACTION='<command>' TARGET='<target>' STATUS='<status>'
```

**Example:**

```
[2025-08-08 10:45:23] USER='alice' ACTION='mkdir' TARGET='docs' STATUS='success'
[2025-08-08 10:46:01] USER='root' ACTION='deluser' TARGET='bob' STATUS='success'
```

---

## ⚙️ How to run this project

Run `./build.sh` to create `./simulator`
```bash
./build.sh
./simulator
```

---

## 🚀 Usage Example

```bash
$ ./simulator
Command> login Alice
Logged in as Alice
command> mkdir Documents
Directory 'Documents' created.
command> cd Documents
command> pwd
/home/Alice/Documents
command> touch file.txt 
File 'file.txt' created.
command> ls
[F] file.txt
command> ls -l
-rw-r--r--  Alice  Alice  file.txt
command> logout
User Alice logged out.
command> login Bob
Logged in as Bob
command> cd ..
command> cd Alice
command> cd Documents
command> write file.txt
Permission denied.

```

---

## 🛡️ Permissions System

* **Owner**: Full control over own files/directories
* **Group**: Access depends on assigned group permissions
* **Others**: Access controlled by last permission digit (e.g., `755`)
* Permissions follow the **rwx** model:

  * Read (4)
  * Write (2)
  * Execute (1)

Example: `755` → Owner `rwx`, Group `r-x`, Others `r-x`

