//Nathaniel Klepeis
//SFML version 2.6.1
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath> // For std::round

using namespace std;


/**
 * @struct Course
 * @brief Represents a single course with its code, name, credits, and grade.
 */
struct Course {
    string courseCode; 
    string courseName;
    int credits;
    string grade;

    double getGradePoints() const {
        static const map<string, double> gradeMap = {
            {"A", 4.0}, {"A+", 4.0}, {"A-", 3.7},
            {"B+", 3.3}, {"B", 3.0}, {"B-", 2.7},
            {"C+", 2.3}, {"C", 2.0}, {"C-", 1.7},
            {"D+", 1.3}, {"D", 1.0}, {"D-", 0.7},
            {"F", 0.0}, {"W", -1.0}, {"P", -1.0} 
        };

        auto it = gradeMap.find(grade);
        if (it != gradeMap.end()) {
            return it->second;
        }
        return 0.0;
    }
};

/**
 * @struct Semester
 * @brief Represents a single semester, containing a list of courses.
 */
struct Semester {
    string semesterID;
    vector<Course> courses;

    double calculateSemesterGPA() const {
        double totalPoints = 0.0;
        int totalCredits = 0;

        for (const auto& course : courses) {
            double points = course.getGradePoints();
            if (points >= 0) {
                totalPoints += points * course.credits;
                totalCredits += course.credits;
            }
        }

        if (totalCredits == 0) {
            return 0.0;
        }
        return totalPoints / totalCredits;
    }

    void sortByCourseNumber() {
        sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
            return a.courseCode < b.courseCode;
        });
    }

    void sortByGrade() {
        sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
            return a.getGradePoints() > b.getGradePoints();
        });
    }

    bool deleteCourse(const string& courseCode) {
        auto it = remove_if(courses.begin(), courses.end(), [&](const Course& course) {
            return course.courseCode == courseCode;
        });

        if (it != courses.end()) {
            courses.erase(it, courses.end());
            return true;
        }
        return false;
    }
};

/**
 * @class Transcript
 * @brief Manages the student's entire academic record.
 */
class Transcript {
public:
    string studentName = "No Student Name Set";
    vector<Semester> semesters;

    Semester* findSemester(const string& semesterID) {
        for (auto& sem : semesters) {
            if (sem.semesterID == semesterID) {
                return &sem;
            }
        }
        return nullptr;
    }

    bool deleteSemester(const string& semesterID) {
        auto it = remove_if(semesters.begin(), semesters.end(), [&](const Semester& sem) {
            return sem.semesterID == semesterID;
        });

        if (it != semesters.end()) {
            semesters.erase(it, semesters.end());
            return true;
        }
        return false;
    }

    double calculateCumulativeGPA() const {
        map<string, pair<string, Course>> latestCourses;

        for (const auto& semester : semesters) {
            for (const auto& course : semester.courses) {
                if (latestCourses.find(course.courseCode) == latestCourses.end()) {
                    latestCourses[course.courseCode] = {semester.semesterID, course};
                } else {
                    string existingSemesterID = latestCourses[course.courseCode].first;
                    if (semester.semesterID > existingSemesterID) {
                        latestCourses[course.courseCode] = {semester.semesterID, course};
                    }
                }
            }
        }

        double totalPoints = 0.0;
        int totalCredits = 0;

        for (const auto& pair : latestCourses) {
            const Course& course = pair.second.second;
            double points = course.getGradePoints();
            
            if (points >= 0) {
                totalPoints += points * course.credits;
                totalCredits += course.credits;
            }
        }

        if (totalCredits == 0) {
            return 0.0;
        }
        return totalPoints / totalCredits;
    }

    void saveToCSV(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) return;

        file << studentName << "\n";

        for (const auto& semester : semesters) {
            for (const auto& course : semester.courses) {
                file << semester.semesterID << ","
                     << course.courseCode << ","
                     << course.courseName << ","
                     << course.credits << ","
                     << course.grade << "\n";
            }
        }
        file.close();
    }

    void loadFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        semesters.clear();
        studentName = "";

        string line;
        if (getline(file, line)) {
            studentName = line;
        }

        while (getline(file, line)) {
            stringstream ss(line);
            string field;
            
            string semID, cCode, cName, sCredits, sGrade;
            int credits;

            try {
                getline(ss, semID, ',');
                getline(ss, cCode, ',');
                getline(ss, cName, ',');
                getline(ss, sCredits, ',');
                getline(ss, sGrade, ',');

                credits = sCredits.empty() ? 0 : stoi(sCredits);

                Course newCourse = {cCode, cName, credits, sGrade};

                Semester* semester = findSemester(semID);
                if (semester == nullptr) {
                    semesters.push_back(Semester{semID, {}});
                    semester = &semesters.back();
                }
                
                semester->courses.push_back(newCourse);
            } catch (const std::exception& e) {
                // Ignore malformed lines
            }
        }

        file.close();
        sort(semesters.begin(), semesters.end(), [](const Semester& a, const Semester& b){
            return a.semesterID < b.semesterID;
        });
    }
};

// SFML UI Components

/**
 * @struct Button
 * @brief Simple interactive button structure for SFML.
 */
struct Button {
    sf::RectangleShape rect;
    sf::Text text;
    bool isActive = true;

    Button(const string& label, const sf::Font& font, float x, float y, float w, float h) {
        rect.setSize({w, h});
        rect.setPosition(x, y);
        rect.setFillColor(sf::Color(75, 125, 250));
        rect.setOutlineThickness(2);
        rect.setOutlineColor(sf::Color(100, 100, 100));

        text.setString(label);
        text.setFont(font);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);

        centerText();
    }

    void centerText() {
        sf::FloatRect textBounds = text.getLocalBounds();
        sf::Vector2f rectPos = rect.getPosition();
        sf::Vector2f rectSize = rect.getSize();
        
        text.setOrigin(textBounds.left + textBounds.width / 2.0f,
                       textBounds.top + textBounds.height / 2.0f);
        text.setPosition(rectPos.x + rectSize.x / 2.0f,
                         rectPos.y + rectSize.y / 2.0f);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(rect);
        window.draw(text);
    }

    bool isClicked(int mouseX, int mouseY) const {
        return isActive && rect.getGlobalBounds().contains((float)mouseX, (float)mouseY);
    }

    void setHover(bool isHovering) {
        if (!isActive) return;
        rect.setFillColor(isHovering ? sf::Color(100, 150, 200) : sf::Color(70, 70, 70));
    }

    void setInactive() {
        isActive = false;
        rect.setFillColor(sf::Color(50, 50, 50));
        text.setFillColor(sf::Color(150, 150, 150));
    }
};

/**
 * @struct InputField
 * @brief Simple single-line text input field for SFML.
 */
struct InputField {
    sf::RectangleShape rect;
    sf::Text display;
    string text = "";
    string placeholder;
    bool isFocused = false;
    bool isNumeric = false;
    
    InputField(const sf::Font& font, float x, float y, float w, float h, const string& ph, bool numeric = false) 
    : placeholder(ph), isNumeric(numeric) {
        rect.setSize({w, h});
        rect.setPosition(x, y);
        rect.setFillColor(sf::Color::White);
        rect.setOutlineThickness(1);
        rect.setOutlineColor(sf::Color(150, 150, 150));

        display.setFont(font);
        display.setCharacterSize(14);
        display.setFillColor(sf::Color::Black);
        display.setPosition(x + 5, y + 5);
        updateDisplay();
    }

    void updateDisplay() {
        if (text.empty() && !isFocused) {
            display.setString(placeholder);
            display.setFillColor(sf::Color(180, 180, 180));
        } else {
            display.setString(text);
            display.setFillColor(sf::Color::Black);
        }
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(rect);
        window.draw(display);
    }

    bool checkClick(int mouseX, int mouseY) {
        bool clicked = rect.getGlobalBounds().contains((float)mouseX, (float)mouseY);
        isFocused = clicked;
        rect.setOutlineColor(isFocused ? sf::Color(0, 150, 255) : sf::Color(150, 150, 150));
        updateDisplay();
        return clicked;
    }

    void processInput(sf::Uint32 unicode) {
        if (!isFocused) return;

        if (unicode == 8) { // Backspace
            if (!text.empty()) {
                text.pop_back();
            }
        } else if (unicode < 128 && unicode != 13) { // Printable characters, not Enter
            char c = static_cast<char>(unicode);
            if (isNumeric) {
                if (isdigit(c)) {
                    text += c;
                }
            } else {
                text += c;
            }
        }
        updateDisplay();
    }
};

/**
 * @class TranscriptApp
 * @brief Main SFML application logic and state manager.
 */
class TranscriptApp {
public:
    enum State {
        STATE_MAIN_MENU,
        STATE_VIEW_SUMMARY,
        STATE_INPUT_STUDENT_NAME,
        STATE_ADD_SEMESTER,
        STATE_DELETE_SEMESTER,
        STATE_SEMESTER_MENU,
        STATE_ADD_COURSE,
        STATE_DELETE_COURSE,
        STATE_LOAD_SAVE_CONFIRM,
        STATE_MESSAGE,
        STATE_MESSAGE_SEM
    };

    TranscriptApp() : 
        window(sf::VideoMode(800, 700), "SFML Transcript Manager"),
        currentState(STATE_MAIN_MENU) 
    {
        window.setFramerateLimit(60);

        // Font Loading
        if (!font.loadFromFile("/usr/share/fonts/truetype/freefont/FreeMono.ttf")) {
            if (!font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf")) {
                 if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
                    cerr << "Error: Could not load font. Please ensure 'arial.ttf' is in the execution directory." << endl;
                    // Fallback to a functional state but with a warning
                }
            }
        }

        setupUI();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    sf::RenderWindow window;
    sf::Font font;
    Transcript transcript;
    State currentState;
    vector<Button> buttons;
    vector<InputField> inputs;
    string currentSemesterID = ""; // Used for STATE_SEMESTER_MENU and course actions
    string messageText = ""; // For STATE_MESSAGE

    // For scrolling the transcript view
    float viewScrollOffset = 0.0f;
    const float maxScroll = 0.0f;

    // UI Setup & Management

    void setupUI() {
        buttons.clear();
        inputs.clear();

        if (currentState == STATE_MAIN_MENU) {
            float x = 50.0f;
            float y = 150.0f;
            float buttonWidth = 250.0f;
            float buttonHeight = 40.0f;
            float spacing = 10.0f;

            buttons.emplace_back("Set Student Name", font, x, y, buttonWidth, buttonHeight);
            buttons.emplace_back("Add New Semester", font, x, y + (buttonHeight + spacing) * 1, buttonWidth, buttonHeight);
            buttons.emplace_back("Delete Semester", font, x, y + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight);
            buttons.emplace_back("View Full Transcript", font, x, y + (buttonHeight + spacing) * 3, buttonWidth, buttonHeight);
            buttons.emplace_back("Save Transcript (transcript.csv)", font, x, y + (buttonHeight + spacing) * 4, buttonWidth, buttonHeight);
            buttons.emplace_back("Load Transcript (transcript.csv)", font, x, y + (buttonHeight + spacing) * 5, buttonWidth, buttonHeight);
            buttons.emplace_back("Exit", font, x, y + (buttonHeight + spacing) * 6, buttonWidth, buttonHeight);

        } else if (currentState == STATE_INPUT_STUDENT_NAME) {
            inputs.emplace_back(font, 50, 200, 300, 30, "Enter Full Name", false);
            buttons.emplace_back("Save Name", font, 50, 250, 145, 30);
            buttons.emplace_back("Back", font, 205, 250, 145, 30);

        } else if (currentState == STATE_ADD_SEMESTER) {
            inputs.emplace_back(font, 50, 200, 300, 30, "Enter Semester ID (e.g., 202540)", true);
            buttons.emplace_back("Add Semester", font, 50, 250, 145, 30);
            buttons.emplace_back("Back", font, 205, 250, 145, 30);
        
        } else if (currentState == STATE_DELETE_SEMESTER) {
            inputs.emplace_back(font, 50, 200, 300, 30, "Enter Semester ID to Delete", true);
            buttons.emplace_back("Delete Semester", font, 50, 250, 145, 30);
            buttons.emplace_back("Back", font, 205, 250, 145, 30);

        } else if (currentState == STATE_SEMESTER_MENU) {
            float x = 50.0f;
            float y = 150.0f;
            float buttonWidth = 250.0f;
            float buttonHeight = 40.0f;
            float spacing = 10.0f;

            buttons.emplace_back("Add Course", font, x, y, buttonWidth, buttonHeight);
            buttons.emplace_back("Delete Course", font, x, y + (buttonHeight + spacing) * 1, buttonWidth, buttonHeight);
            buttons.emplace_back("View/Sort Courses", font, x, y + (buttonHeight + spacing) * 2, buttonWidth, buttonHeight);
            buttons.emplace_back("Back to Main Menu", font, x, y + (buttonHeight + spacing) * 3, buttonWidth, buttonHeight);

        } else if (currentState == STATE_ADD_COURSE) {
            inputs.emplace_back(font, 50, 150, 300, 30, "Course Code (e.g., CSC 319)", false);
            inputs.emplace_back(font, 50, 200, 300, 30, "Course Name", false);
            inputs.emplace_back(font, 50, 250, 300, 30, "Credits (e.g., 3)", true);
            inputs.emplace_back(font, 50, 300, 300, 30, "Grade (e.g., A, B+, F)", false);

            buttons.emplace_back("Add Course", font, 50, 350, 145, 30);
            buttons.emplace_back("Back", font, 205, 350, 145, 30);

        } else if (currentState == STATE_DELETE_COURSE) {
            inputs.emplace_back(font, 50, 200, 300, 30, "Enter Course Code to Delete", false);
            buttons.emplace_back("Delete Course", font, 50, 250, 145, 30);
            buttons.emplace_back("Back", font, 205, 250, 145, 30);
        }
    }

    // Event Handling 

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::TextEntered) {
                for (auto& input : inputs) {
                    input.processInput(event.text.unicode);
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handleMouseClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                // Handle scrolling in View Summary state
                if (currentState == STATE_VIEW_SUMMARY && event.mouseWheelScroll.delta != 0) {
                    viewScrollOffset += event.mouseWheelScroll.delta * 25.0f;
                    // Clamp the offset
                    viewScrollOffset = max(viewScrollOffset, 0.0f);
                }
            }
        }
    }

    void handleMouseClick(int x, int y) {
        // Check input fields first for focus
        for (auto& input : inputs) {
            input.checkClick(x, y);
        }

        // Handle button clicks based on state
        if (currentState == STATE_MAIN_MENU) {
            if (buttons[0].isClicked(x, y)) { // Set Student Name
                setState(STATE_INPUT_STUDENT_NAME);
            } else if (buttons[1].isClicked(x, y)) { // Add Semester
                setState(STATE_ADD_SEMESTER);
            } else if (buttons[2].isClicked(x, y)) { // Delete Semester
                setState(STATE_DELETE_SEMESTER);
            } else if (buttons[3].isClicked(x, y)) { // View Full Transcript
                viewScrollOffset = 0.0f; // Reset scroll
                currentSemesterID = "ALL";
                setState(STATE_VIEW_SUMMARY);
            } else if (buttons[4].isClicked(x, y)) { // Save
                transcript.saveToCSV("transcript.csv");
                setMessage("Transcript saved to transcript.csv!");
            } else if (buttons[5].isClicked(x, y)) { // Load
                transcript.loadFromCSV("transcript.csv");
                setMessage("Transcript loaded from transcript.csv!");
            } else if (buttons[6].isClicked(x, y)) { // Exit
                window.close();
            } else if (Semester* sem = checkSemesterClick(x, y)) { // Clicked on a semester name in the main menu
                currentSemesterID = sem->semesterID;
                setState(STATE_SEMESTER_MENU);
            }

        } else if (currentState == STATE_INPUT_STUDENT_NAME) {
            if (buttons[0].isClicked(x, y)) { // Save Name
                transcript.studentName = inputs[0].text.empty() ? "No Student Name Set" : inputs[0].text;
                setMessage("Student name updated to: " + transcript.studentName);
            } else if (buttons[1].isClicked(x, y)) { // Back
                setState(STATE_MAIN_MENU);
            }

        } else if (currentState == STATE_ADD_SEMESTER) {
            if (buttons[0].isClicked(x, y)) { // Add Semester
                if (!inputs[0].text.empty()) {
                    if (transcript.findSemester(inputs[0].text) == nullptr) {
                        transcript.semesters.push_back(Semester{inputs[0].text, {}});
                        // Sort semesters by ID
                        sort(transcript.semesters.begin(), transcript.semesters.end(), [](const Semester& a, const Semester& b){
                            return a.semesterID < b.semesterID;
                        });
                        setMessage("Semester " + inputs[0].text + " added.");
                    } else {
                        setMessage("Error: Semester " + inputs[0].text + " already exists!");
                    }
                } else {
                    setMessage("Error: Semester ID cannot be empty.");
                }
            } else if (buttons[1].isClicked(x, y)) { // Back
                setState(STATE_MAIN_MENU);
            }
        
        } else if (currentState == STATE_DELETE_SEMESTER) {
            if (buttons[0].isClicked(x, y)) { // Delete Semester
                if (transcript.deleteSemester(inputs[0].text)) {
                    setMessage("Semester " + inputs[0].text + " deleted.");
                } else {
                    setMessage("Error: Semester " + inputs[0].text + " not found!");
                }
            } else if (buttons[1].isClicked(x, y)) { // Back
                setState(STATE_MAIN_MENU);
            }
        
        } else if (currentState == STATE_VIEW_SUMMARY) {
            if (buttons[0].isClicked(x, y)) { // Back
                setState(STATE_MAIN_MENU);
            } else if (Semester* sem = checkSemesterClick(x, y)) { // Clicked on a semester name in the summary
                currentSemesterID = sem->semesterID;
                setState(STATE_SEMESTER_MENU);
            }

        } else if (currentState == STATE_SEMESTER_MENU) {
            if (buttons[0].isClicked(x, y)) { // Add Course
                setState(STATE_ADD_COURSE);
            } else if (buttons[1].isClicked(x, y)) { // Delete Course
                setState(STATE_DELETE_COURSE);
            } else if (buttons[2].isClicked(x, y)) { // View/Sort Courses
                viewScrollOffset = 0.0f;
                setState(STATE_VIEW_SUMMARY); // Uses the same view, but will focus on the single semester
            } else if (buttons[3].isClicked(x, y)) { // Back
                setState(STATE_MAIN_MENU);
            }

        } else if (currentState == STATE_ADD_COURSE) {
            if (buttons[0].isClicked(x, y)) { // Add Course
                Semester* sem = transcript.findSemester(currentSemesterID);
                if (sem && !inputs[0].text.empty() && !inputs[2].text.empty() && !inputs[3].text.empty()) {
                    try {
                        Course newCourse = {
                            inputs[0].text, // Code
                            inputs[1].text, // Name (can be empty)
                            stoi(inputs[2].text), // Credits
                            inputs[3].text  // Grade
                        };
                        sem->courses.push_back(newCourse);
                        setMessageSem("Course " + newCourse.courseCode + " added to " + currentSemesterID);
                    } catch (...) {
                        setMessageSem("Error: Invalid input for Credits.");
                    }
                } else {
                    setMessageSem("Error: Course Code, Credits, and Grade are required.");
                }
            } else if (buttons[1].isClicked(x, y)) { // Back
                setState(STATE_SEMESTER_MENU);
            }

        } else if (currentState == STATE_DELETE_COURSE) {
            if (buttons[0].isClicked(x, y)) { // Delete Course
                Semester* sem = transcript.findSemester(currentSemesterID);
                if (sem) {
                    if (sem->deleteCourse(inputs[0].text)) {
                        setMessage("Course " + inputs[0].text + " deleted from " + currentSemesterID);
                    } else {
                        setMessage("Error: Course " + inputs[0].text + " not found in semester " + currentSemesterID);
                    }
                }
            } else if (buttons[1].isClicked(x, y)) { // Back
                setState(STATE_SEMESTER_MENU);
            }
        } else if (currentState == STATE_MESSAGE) {
            if (buttons[0].isClicked(x, y)) { // OK/Back
                setState(STATE_MAIN_MENU);
            }
        } else if (currentState == STATE_MESSAGE_SEM) {
            if (buttons[0].isClicked(x, y)) { // OK/Back
                setState(STATE_SEMESTER_MENU);
            }
        }
    }

    // Checks if a click occurred on a Semester ID in the display area
    Semester* checkSemesterClick(int mouseX, int mouseY) {
        float currentY = 0.0f; 
        
        if (currentState == STATE_MAIN_MENU) {
            currentY = 525.0f; // Semester list starts here in Main Menu
        } else if (currentState == STATE_VIEW_SUMMARY) {
            currentY = 200.0f + ((1.35) * viewScrollOffset);
        } else {
            return nullptr;
        }

        const float rowHeight = 25.0f;
        const float xStart = 50.0f;
        const float xEnd = 750.0f; 

        if (mouseX < xStart || mouseX > xEnd) return nullptr;

        for (auto& sem : transcript.semesters) {
            if (mouseY > currentY && mouseY < currentY + rowHeight) {
                return &sem;
            }
            if (currentState == STATE_MAIN_MENU) {
                currentY += rowHeight;
            }
            // In summary view, skip past course details
            if (currentState == STATE_VIEW_SUMMARY) {
                currentY += (sem.courses.size() + 2) * rowHeight;
            }
        }
        return nullptr;
    }

    void setState(State newState) {
        currentState = newState;
        setupUI();
    }

    void setMessage(const string& msg) {
        messageText = msg;
        setState(STATE_MESSAGE);
        buttons.clear();
        buttons.emplace_back("OK", font, 350, 400, 100, 40);
    }

    void setMessageSem(const string& msg) {
        messageText = msg;
        setState(STATE_MESSAGE_SEM);
        buttons.clear();
        buttons.emplace_back("OK", font, 350, 400, 100, 40);
    }

    // Rendering

    void update() {
        // Handle button hover effects
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        for (auto& btn : buttons) {
            btn.setHover(btn.isClicked(mousePos.x, mousePos.y));
        }
    }

    void render() {
        window.clear(sf::Color(175, 150, 150)); // Dark background

        sf::Text title;
        title.setFont(font);
        title.setCharacterSize(24);
        title.setFillColor(sf::Color::White);
        title.setPosition(50, 20);

        sf::Text subtitle;
        subtitle.setFont(font);
        subtitle.setCharacterSize(18);
        subtitle.setFillColor(sf::Color(200, 200, 200));
        subtitle.setPosition(50, 60);

        if (currentState == STATE_MAIN_MENU) {
            title.setString("Transcript Manager: Main Menu");
            subtitle.setString("Student: " + transcript.studentName + " | Cumulative GPA: " + 
                                to_string(round(transcript.calculateCumulativeGPA() * 100) / 100.0f));
            window.draw(title);
            window.draw(subtitle);
            
            // List of semesters (for quick access)
            float y = 500.0f;
            drawText(window, "Existing Semesters (Click to Enter):", 50, y, 16, sf::Color::Yellow);
            y += 30;
            for (const auto& sem : transcript.semesters) {
                drawText(window, sem.semesterID, 50, y, 14, sf::Color::White);
                y += 25;
            }

        } else if (currentState == STATE_VIEW_SUMMARY) {
            if (!currentSemesterID.empty() && currentSemesterID != "ALL") {
                // Viewing a single semester
                title.setString("Semester Details: " + currentSemesterID);
                subtitle.setString("Click on 'Back' or a semester ID to return.");
            } else {
                // Viewing the full transcript
                title.setString("Full Transcript Summary");
                subtitle.setString("Student: " + transcript.studentName + " | Cumulative GPA: " + 
                                to_string(round(transcript.calculateCumulativeGPA() * 100) / 100.0f));
                currentSemesterID = "ALL"; // Clear focus
            }
            window.draw(title);
            window.draw(subtitle);
            
            drawSummary(window);

            buttons.emplace_back("Back to Main Menu", font, 50, 650, 150, 30); // Back button below scroll area

        } else if (currentState == STATE_INPUT_STUDENT_NAME) {
            title.setString("Enter Student Name");
            window.draw(title);

        } else if (currentState == STATE_ADD_SEMESTER) {
            title.setString("Add New Semester");
            window.draw(title);

        } else if (currentState == STATE_DELETE_SEMESTER) {
            title.setString("Delete Semester");
            window.draw(title);

        } else if (currentState == STATE_SEMESTER_MENU) {
            title.setString("Semester Manager: " + currentSemesterID);
            
            Semester* sem = transcript.findSemester(currentSemesterID);
            if (sem) {
                float gpa = round(sem->calculateSemesterGPA() * 100) / 100.0f;
                subtitle.setString("Semester GPA: " + to_string(gpa));
            } else {
                subtitle.setString("Error: Semester not found.");
            }
            window.draw(title);
            window.draw(subtitle);

        } else if (currentState == STATE_ADD_COURSE) {
            title.setString("Add Course to " + currentSemesterID);
            window.draw(title);
            drawText(window, "Note: Credits must be a whole number (e.g., 3).", 50, 100, 14, sf::Color(255, 150, 150));

        } else if (currentState == STATE_DELETE_COURSE) {
            title.setString("Delete Course from " + currentSemesterID);
            window.draw(title);
        } else if (currentState == STATE_MESSAGE) {
             title.setString("Notification");
             window.draw(title);
             drawText(window, messageText, 50, 200, 18, sf::Color::Cyan);
        } else if (currentState == STATE_MESSAGE_SEM) {
             title.setString("Notification");
             window.draw(title);
             drawText(window, messageText, 50, 200, 18, sf::Color::Cyan);
        }

        // Draw general UI elements (buttons/inputs)
        for (const auto& btn : buttons) {
            btn.draw(window);
        }
        for (const auto& input : inputs) {
            input.draw(window);
        }
        
        window.display();
    }

    void drawSummary(sf::RenderWindow& window) {
        float currentY = 150.0f + viewScrollOffset;
        const float rowHeight = 25.0f;
        
        // Define the area where content can be drawn (to hide content that scrolls off screen)
        sf::View view(sf::FloatRect(0, 0, 800, 700)); // Default view
        window.setView(view);

        // Clip/Scroll Area Rectangle (50, 100) to (750, 600)
        sf::RectangleShape clipRect({700, 480});
        clipRect.setPosition(50, 100);
        clipRect.setFillColor(sf::Color(40, 40, 40));
        window.draw(clipRect);

        // Adjust view to enable scrolling within the visible area
        sf::View scrollableView = window.getView();
        scrollableView.setViewport(sf::FloatRect(50.0f / 800.0f, 100.0f / 700.0f, 700.0f / 800.0f, 500.0f / 700.0f));
        scrollableView.setCenter(400, 350 - viewScrollOffset);
        window.setView(scrollableView);


        const vector<Semester>* semestersToDisplay = &transcript.semesters;
        if (currentSemesterID != "ALL") {
            Semester* sem = transcript.findSemester(currentSemesterID);
            if (sem) {
                semestersToDisplay = new vector<Semester>{*sem};
            }
        }

        for (const auto& semester : *semestersToDisplay) {
            // Semester Header
            drawText(window, "--- Semester: " + semester.semesterID + " ---", 50, currentY, 18, sf::Color::Yellow);
            currentY += rowHeight;

            // Column Headers
            drawTable(window, currentY, "Course", "Name", "Credits", "Grade", sf::Color(150, 150, 150));
            currentY += rowHeight;
            
            // Course Rows
            for (const auto& course : semester.courses) {
                drawTable(window, currentY, 
                          course.courseCode, 
                          course.courseName.length() > 25 ? course.courseName.substr(0, 22) + "..." : course.courseName, // Truncate long names
                          to_string(course.credits), 
                          course.grade,
                          sf::Color::White);
                currentY += rowHeight;
            }

            // Semester GPA
            float gpa = round(semester.calculateSemesterGPA() * 100) / 100.0f;
            drawText(window, "Semester GPA: " + to_string(gpa), 50, currentY, 16, sf::Color::Green);
            currentY += rowHeight * 1.5f;
        }

        // Restore original view for drawing elements outside the scroll area
        window.setView(view);

        // Draw button outside the scroll area
        buttons.clear();
        buttons.emplace_back("Back to Main Menu", font, 50, 650, 150, 30);
    }

    // Helper to draw a single line of text with custom color/size
    void drawText(sf::RenderWindow& window, const string& str, float x, float y, unsigned int size, const sf::Color& color) {
        sf::Text txt(str, font, size);
        txt.setFillColor(color);
        txt.setPosition(x, y);
        window.draw(txt);
    }

    // Helper to draw a formatted table row
    void drawTable(sf::RenderWindow& window, float y, const string& col1, const string& col2, const string& col3, const string& col4, const sf::Color& color) {
        drawText(window, col1, 50, y, 14, color);     // Course Code
        drawText(window, col2, 170, y, 14, color);    // Name
        drawText(window, col3, 470, y, 14, color);    // Credits
        drawText(window, col4, 570, y, 14, color);    // Grade
    }
};

// Main Function

int main() {
    // Prevent console output from the original code
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    TranscriptApp app;
    app.run();

    return 0;
}