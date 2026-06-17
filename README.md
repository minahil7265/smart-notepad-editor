# Smart-Notepad-Editor
A feature-rich console-based text editor built in C++ supporting file handling, editing, formatting (font, size, color), navigation tools, and find &amp; replace functionality.

## 🚀 Features

### 📁 File Management
- Create new files
- Open existing files
- Save & Save As functionality
- Recent files history (up to 5 files)
- Close file safely with unsaved changes warning

### ✏️ Editing Tools
- Type text line by line
- Edit, delete, and insert lines
- Find and replace text
- Select all document
- Navigation (Home, End, Page Up, Page Down)

### 🎨 Formatting Options
- Change font (simulated)
- Adjust font size (simulated)
- Change text color (console-based representation)

### 📄 Document Display
- Clean formatted console view
- Line numbering
- Modified file indicator
- Auto-wrapped line display

---

## 🛠️ Technologies Used
- C++
- File Handling (`fstream`)
- STL (vector, string, algorithm)
- Console UI (ASCII formatting)

---

## 📂 How to Run

### Compile:
```bash
g++ main.cpp -o editor
