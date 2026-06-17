#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib> 
using namespace std;


struct EditorState {
    vector<string> lines;
    string         currentFile;
    bool           modified;
    string         font;
    int            fontSize;
    string         textColor;
    vector<string> recentFiles;

    EditorState()
        : modified(false), font("Arial"), fontSize(12), textColor("Black") {
        lines.push_back("");
    }
};

EditorState editor;

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        cout << "\033[2J\033[1;1H";
    #endif
}

void pauseScreen() {
    cout << "\n  Press ENTER to continue...";
    cin.ignore(1000, '\n');
}

void printLine(int width = 60) {
    cout << "  ";
    for (int i = 0; i < width; i++) cout << '-';
    cout << "\n";
}

void printHeader(const string& title, int width = 60) {
    printLine(width);
    int pad = (width - (int)title.size()) / 2;
    cout << "  ";
    for (int i = 0; i < pad; i++) cout << ' ';
    cout << title << "\n";
    printLine(width);
}

void addToRecent(const string& path) {
    editor.recentFiles.erase(
        remove(editor.recentFiles.begin(), editor.recentFiles.end(), path),
        editor.recentFiles.end());
    editor.recentFiles.insert(editor.recentFiles.begin(), path);
    if (editor.recentFiles.size() > 5)
        editor.recentFiles.resize(5);
}


void renderDocument() {
    clearScreen();
    string fname = editor.currentFile.empty() ? "[Untitled]" : editor.currentFile;

    printLine(62);
    cout << "  File  : " << fname << (editor.modified ? "  [Modified]" : "") << "\n";
    cout << "  Font  : " << editor.font
         << "   Size: " << editor.fontSize << "pt"
         << "   Color: " << editor.textColor << "\n";
    printLine(62);

    if (editor.lines.empty() || (editor.lines.size() == 1 && editor.lines[0].empty())) {
        cout << "  (empty document)\n";
    } else {
        for (size_t i = 0; i < editor.lines.size(); i++) {
            string line = editor.lines[i];
            if (line.empty()) {
                cout << "  [Para " << i + 1 << "] (empty line)\n";
                continue;
            }
            cout << "  [" << i + 1 << "] ";
            while (!line.empty()) {
                string chunk = line.substr(0, 55);
                line = line.size() > 55 ? line.substr(55) : "";
                cout << chunk << "\n";
                if (!line.empty()) cout << "       ";
            }
        }
    }
    printLine(62);
}


void newFile() {
    if (editor.modified) {
        cout << "\n  Unsaved changes! Save first? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c == 'y' || c == 'Y') {
            if (editor.currentFile.empty()) {
                cout << "  Enter filename: ";
                getline(cin, editor.currentFile);
            }
            ofstream f(editor.currentFile);
            for (auto& l : editor.lines) f << l << "\n";
            f.close();
            addToRecent(editor.currentFile);
        }
    }
    editor.lines.clear();
    editor.lines.push_back("");
    editor.currentFile = "";
    editor.modified = false;
    cout << "  New file created.\n";
}

void saveFile(bool saveAs = false) {
    if (saveAs || editor.currentFile.empty()) {
        cout << "  Enter filename (with .txt): ";
        getline(cin, editor.currentFile);
    }
    ofstream f(editor.currentFile);
    if (!f) { cout << "  ERROR: Cannot open file for writing.\n"; return; }
    for (auto& l : editor.lines) f << l << "\n";
    f.close();
    editor.modified = false;
    addToRecent(editor.currentFile);
    cout << "  Saved to \"" << editor.currentFile << "\"\n";
}

void openFile() {
    cout << "\n";
    printHeader("Recent Files");
    if (editor.recentFiles.empty()) {
        cout << "  (none)\n";
    } else {
        for (size_t i = 0; i < editor.recentFiles.size(); i++)
            cout << "  " << i + 1 << ".  " << editor.recentFiles[i] << "\n";
    }
    printLine();
    cout << "  Enter filename (or number from recent list): ";
    string input; getline(cin, input);

    string path = input;
    bool isNum = !input.empty();
    for (char c : input) if (!isdigit(c)) { isNum = false; break; }
    if (isNum) {
        int idx = stoi(input) - 1;
        if (idx >= 0 && idx < (int)editor.recentFiles.size())
            path = editor.recentFiles[idx];
        else { cout << "  ERROR: Invalid selection.\n"; return; }
    }

    ifstream f(path);
    if (!f) { cout << "  ERROR: Cannot open \"" << path << "\"\n"; return; }
    editor.lines.clear();
    string line;
    while (getline(f, line)) editor.lines.push_back(line);
    if (editor.lines.empty()) editor.lines.push_back("");
    f.close();
    editor.currentFile = path;
    editor.modified = false;
    addToRecent(path);
    cout << "  Opened \"" << path << "\"\n";
}

void closeFile() {
    if (editor.modified) {
        cout << "  Unsaved changes! Save? (y/n): ";
        char c; cin >> c; cin.ignore();
        if (c == 'y' || c == 'Y') saveFile();
    }
    editor.lines.clear();
    editor.lines.push_back("");
    editor.currentFile = "";
    editor.modified = false;
    cout << "  File closed.\n";
}



void typeText() {
    renderDocument();
    cout << "\n";
    printHeader("Type Text Mode");
    cout << "  - Each line you enter becomes a new paragraph.\n";
    cout << "  - Press ENTER on empty line = blank paragraph.\n";
    cout << "  - Type 'DONE' to finish.\n";
    printLine();

    string input;
    while (true) {
        cout << "  > ";
        getline(cin, input);
        if (input == "DONE") break;
        editor.lines.push_back(input);
        editor.modified = true;
    }
    cout << "  Text added.\n";
}

void editText() {
    renderDocument();
    cout << "\n";
    printHeader("Edit Mode");
    cout << "  1. Delete a line\n";
    cout << "  2. Edit (overwrite) a line\n";
    cout << "  3. Insert a line before another\n";
    printLine();
    cout << "  Choice: ";
    int c; cin >> c; cin.ignore();

    if (editor.lines.empty()) { cout << "  Document is empty.\n"; return; }

    int lineNo;
    cout << "  Line number (1-" << editor.lines.size() << "): ";
    cin >> lineNo; cin.ignore();
    lineNo--;
    if (lineNo < 0 || lineNo >= (int)editor.lines.size()) {
        cout << "  ERROR: Invalid line number.\n"; return;
    }

    if (c == 1) {
        editor.lines.erase(editor.lines.begin() + lineNo);
        if (editor.lines.empty()) editor.lines.push_back("");
        editor.modified = true;
        cout << "  Line deleted.\n";
    } else if (c == 2) {
        cout << "  Current : " << editor.lines[lineNo] << "\n";
        cout << "  New text: "; string newLine; getline(cin, newLine);
        editor.lines[lineNo] = newLine;
        editor.modified = true;
        cout << "  Line updated.\n";
    } else if (c == 3) {
        cout << "  Insert text: "; string ins; getline(cin, ins);
        editor.lines.insert(editor.lines.begin() + lineNo, ins);
        editor.modified = true;
        cout << "  Line inserted.\n";
    }
}

void changeColor() {
    string colors[] = {"Black","Red","Blue","Green","White","Yellow","Cyan","Magenta"};
    cout << "\n";
    printHeader("Text Colors");
    for (int i = 0; i < 8; i++)
        cout << "  " << i + 1 << ".  " << colors[i] << "\n";
    printLine();
    cout << "  Choice: "; int c; cin >> c; cin.ignore();
    if (c >= 1 && c <= 8) {
        editor.textColor = colors[c - 1];
        editor.modified = true;
        cout << "  Color set to: " << editor.textColor << "\n";
    } else cout << "  ERROR: Invalid choice.\n";
}

void changeFont() {
    string fonts[] = {"Arial","Times New Roman","Courier New","Verdana",
                      "Georgia","Trebuchet MS","Comic Sans MS","Calibri"};
    cout << "\n";
    printHeader("Fonts");
    for (int i = 0; i < 8; i++)
        cout << "  " << i + 1 << ".  " << fonts[i] << "\n";
    printLine();
    cout << "  Choice: "; int c; cin >> c; cin.ignore();
    if (c >= 1 && c <= 8) {
        editor.font = fonts[c - 1];
        editor.modified = true;
        cout << "  Font set to: " << editor.font << "\n";
    } else cout << "  ERROR: Invalid choice.\n";
}

void changeFontSize() {
    int sizes[] = {8, 10, 12, 14, 16, 18, 20, 24, 28, 32, 36, 48, 72};
    cout << "\n";
    printHeader("Font Sizes");
    for (int i = 0; i < 13; i++)
        cout << "  " << i + 1 << ".  " << sizes[i] << "pt\n";
    printLine();
    cout << "  Choice: "; int c; cin >> c; cin.ignore();
    if (c >= 1 && c <= 13) {
        editor.fontSize = sizes[c - 1];
        editor.modified = true;
        cout << "  Font size set to: " << editor.fontSize << "pt\n";
    } else cout << "  ERROR: Invalid choice.\n";
}

void findAndReplace() {
    cout << "\n";
    printHeader("Find and Replace");
    cout << "  Find    : "; string find; getline(cin, find);
    cout << "  Replace : "; string repl; getline(cin, repl);
    int count = 0;
    for (auto& line : editor.lines) {
        size_t pos = 0;
        while ((pos = line.find(find, pos)) != string::npos) {
            line.replace(pos, find.size(), repl);
            pos += repl.size();
            count++;
        }
    }
    if (count) {
        editor.modified = true;
        cout << "  Replaced " << count << " occurrence(s).\n";
    } else {
        cout << "  \"" << find << "\" not found.\n";
    }
}

void selectAllText() {
    cout << "\n";
    printHeader("Select All - Full Document");
    for (size_t i = 0; i < editor.lines.size(); i++)
        cout << "  " << i + 1 << ":  " << editor.lines[i] << "\n";
    printLine();
    cout << "  Total lines selected: " << editor.lines.size() << "\n";
}

void navigationKeys() {
    cout << "\n";
    printHeader("Navigation");
    cout << "  1.  Home   - Go to first line\n";
    cout << "  2.  End    - Go to last line\n";
    cout << "  3.  Pg Up  - Show first 5 lines\n";
    cout << "  4.  Pg Dn  - Show last 5 lines\n";
    printLine();
    cout << "  Choice: "; int c; cin >> c; cin.ignore();

    int total = (int)editor.lines.size();
    printLine();
    if (c == 1) {
        cout << "  [HOME]  First line:\n";
        cout << "  " << (total ? editor.lines[0] : "(empty)") << "\n";
    } else if (c == 2) {
        cout << "  [END]   Last line:\n";
        cout << "  " << (total ? editor.lines[total - 1] : "(empty)") << "\n";
    } else if (c == 3) {
        cout << "  [PG UP] First 5 lines:\n";
        for (int i = 0; i < min(5, total); i++)
            cout << "  " << i + 1 << ":  " << editor.lines[i] << "\n";
    } else if (c == 4) {
        cout << "  [PG DN] Last 5 lines:\n";
        int start = max(0, total - 5);
        for (int i = start; i < total; i++)
            cout << "  " << i + 1 << ":  " << editor.lines[i] << "\n";
    }
    printLine();
}


void fileMenu() {
    int choice;
    do {
        cout << "\n";
        printHeader("File Menu");
        cout << "  1.  New File\n";
        cout << "  2.  Open File / Recent Files\n";
        cout << "  3.  Save\n";
        cout << "  4.  Save As\n";
        cout << "  5.  Close File\n";
        cout << "  0.  Back\n";
        printLine();
        cout << "  Choice: "; cin >> choice; cin.ignore();
        switch (choice) {
        case 1: newFile();      break;
        case 2: openFile();     break;
        case 3: saveFile();     break;
        case 4: saveFile(true); break;
        case 5: closeFile();    break;
        case 0: break;
        default: cout << "  Invalid choice.\n";
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

void editMenu() {
    int choice;
    do {
        cout << "\n";
        printHeader("Edit Menu");
        cout << "  1.  Type / Enter Text\n";
        cout << "  2.  Edit / Delete / Insert Line\n";
        cout << "  3.  Find and Replace\n";
        cout << "  4.  Select All\n";
        cout << "  5.  Navigation (Home / End / Pg Up / Pg Dn)\n";
        cout << "  0.  Back\n";
        printLine();
        cout << "  Choice: "; cin >> choice; cin.ignore();
        switch (choice) {
        case 1: typeText();       break;
        case 2: editText();       break;
        case 3: findAndReplace(); break;
        case 4: selectAllText();  break;
        case 5: navigationKeys(); break;
        case 0: break;
        default: cout << "  Invalid choice.\n";
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

void formatMenu() {
    int choice;
    do {
        cout << "\n";
        printHeader("Format Menu");
        cout << "  1.  Change Text Color\n";
        cout << "  2.  Change Font\n";
        cout << "  3.  Change Font Size\n";
        cout << "  0.  Back\n";
        printLine();
        cout << "  Choice: "; cin >> choice; cin.ignore();
        switch (choice) {
        case 1: changeColor();    break;
        case 2: changeFont();     break;
        case 3: changeFontSize(); break;
        case 0: break;
        default: cout << "  Invalid choice.\n";
        }
        if (choice != 0) pauseScreen();
    } while (choice != 0);
}

void mainMenu() {
    int choice;
    do {
        renderDocument();
        cout << "\n";
        printHeader("Console Text Editor - Main Menu");
        cout << "  1.  File Menu\n";
        cout << "  2.  Edit Menu\n";
        cout << "  3.  Format Menu\n";
        cout << "  0.  Exit\n";
        printLine();
        cout << "  Choice: "; cin >> choice; cin.ignore();
        switch (choice) {
        case 1: fileMenu();   break;
        case 2: editMenu();   break;
        case 3: formatMenu(); break;
        case 0:
            if (editor.modified) {
                cout << "  Unsaved changes. Save before exit? (y/n): ";
                char c; cin >> c; cin.ignore();
                if (c == 'y' || c == 'Y') saveFile();
            }
            cout << "  Goodbye!\n";
            break;
        default: cout << "  Invalid choice.\n";
        }
    } while (choice != 0);
}

int main() {
    mainMenu();
    return 0;
}