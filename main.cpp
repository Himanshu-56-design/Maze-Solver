#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <string>
#include <algorithm>
#include <limits>
#include <chrono>
#include <random>

using namespace std;
using namespace chrono;

struct Cell {
    int row;
    int col;
};

struct Result {
    bool found;
    int pathLength;
    int nodesVisited;
    double timeTaken;
    vector<string> outputMaze;
};

class MazeSolver {
    vector<string> maze;
    int rows;
    int cols;
    Cell start;
    Cell finish;

    bool isInside(int r, int c) const {
        return r >= 0 && r < rows && c >= 0 && c < cols;
    }

    bool isOpen(int r, int c) const {
        return isInside(r, c) && maze[r][c] != '#';
    }

    vector<Cell> buildPath(vector<vector<Cell>>& parent, Cell endCell) {
        vector<Cell> path;
        Cell current = endCell;

        while (!(current.row == -1 && current.col == -1)) {
            path.push_back(current);
            current = parent[current.row][current.col];
        }

        reverse(path.begin(), path.end());
        return path;
    }

    vector<string> prepareOutputMaze(vector<vector<bool>>& visited, vector<Cell>& path) {
        vector<string> output = maze;

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (visited[i][j] && output[i][j] == '.') {
                    output[i][j] = 'v';
                }
            }
        }

        for (Cell cell : path) {
            if (output[cell.row][cell.col] != 'S' && output[cell.row][cell.col] != 'E') {
                output[cell.row][cell.col] = '*';
            }
        }

        return output;
    }

public:
    MazeSolver() {
        rows = 0;
        cols = 0;
        start = {-1, -1};
        finish = {-1, -1};
    }

    void loadDefaultMaze() {
        maze = {
            "S..#......",
            "##.#.####.",
            "...#....#.",
            ".#####.#..",
            ".#.....#.#",
            ".#.#####.#",
            ".#.......#",
            ".#######.#",
            ".........E"
        };

        rows = maze.size();
        cols = maze[0].size();
        findStartAndFinish();
    }

    bool loadCustomMaze() {
        int r, c;
        cout << "Enter number of rows: ";
        cin >> r;
        cout << "Enter number of columns: ";
        cin >> c;

        if (r <= 1 || c <= 1) {
            cout << "Maze size must be greater than 1 x 1.\n";
            return false;
        }

        vector<string> newMaze(r);

        cout << "\nUse these symbols:\n";
        cout << "S = start, E = end, . = open path, # = wall\n\n";

        for (int i = 0; i < r; i++) {
            cout << "Enter row " << i + 1 << ": ";
            cin >> newMaze[i];

            if ((int)newMaze[i].size() != c) {
                cout << "Each row must contain exactly " << c << " characters.\n";
                return false;
            }

            for (char ch : newMaze[i]) {
                if (ch != 'S' && ch != 'E' && ch != '.' && ch != '#') {
                    cout << "Invalid character found in maze.\n";
                    return false;
                }
            }
        }

        maze = newMaze;
        rows = r;
        cols = c;

        if (!findStartAndFinish()) {
            cout << "Maze must contain exactly one S and one E.\n";
            loadDefaultMaze();
            return false;
        }

        return true;
    }

    void generateRandomMaze() {
        int r, c, wallPercentage;

        cout << "Enter number of rows: ";
        cin >> r;
        cout << "Enter number of columns: ";
        cin >> c;
        cout << "Enter wall percentage from 10 to 45: ";
        cin >> wallPercentage;

        if (r < 5 || c < 5) {
            cout << "Random maze size must be at least 5 x 5.\n";
            return;
        }

        if (wallPercentage < 10) {
            wallPercentage = 10;
        }

        if (wallPercentage > 45) {
            wallPercentage = 45;
        }

        rows = r;
        cols = c;
        maze.assign(rows, string(cols, '.'));

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> distribution(1, 100);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (distribution(gen) <= wallPercentage) {
                    maze[i][j] = '#';
                }
            }
        }

        start = {0, 0};
        finish = {rows - 1, cols - 1};
        maze[start.row][start.col] = 'S';
        maze[finish.row][finish.col] = 'E';

        for (int i = 0; i < rows; i++) {
            maze[i][0] = '.';
        }

        for (int j = 0; j < cols; j++) {
            maze[rows - 1][j] = '.';
        }

        maze[start.row][start.col] = 'S';
        maze[finish.row][finish.col] = 'E';

        cout << "Random maze generated successfully.\n";
    }

    bool findStartAndFinish() {
        int startCount = 0;
        int finishCount = 0;
        start = {-1, -1};
        finish = {-1, -1};

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (maze[i][j] == 'S') {
                    start = {i, j};
                    startCount++;
                } else if (maze[i][j] == 'E') {
                    finish = {i, j};
                    finishCount++;
                }
            }
        }

        return startCount == 1 && finishCount == 1;
    }

    void displayMaze(const vector<string>& currentMaze) const {
        cout << '\n';
        for (const string& row : currentMaze) {
            cout << row << '\n';
        }
    }

    void displayOriginalMaze() const {
        cout << "\nOriginal Maze:\n";
        displayMaze(maze);
    }

    Result solveUsingBFS() {
        auto begin = high_resolution_clock::now();

        vector<vector<bool>> visited(rows, vector<bool>(cols, false));
        vector<vector<Cell>> parent(rows, vector<Cell>(cols, {-1, -1}));
        queue<Cell> q;
        vector<Cell> path;
        int nodesVisited = 0;

        int dr[] = {-1, 0, 1, 0};
        int dc[] = {0, 1, 0, -1};

        q.push(start);
        visited[start.row][start.col] = true;

        while (!q.empty()) {
            Cell current = q.front();
            q.pop();
            nodesVisited++;

            if (current.row == finish.row && current.col == finish.col) {
                path = buildPath(parent, finish);
                break;
            }

            for (int i = 0; i < 4; i++) {
                int nr = current.row + dr[i];
                int nc = current.col + dc[i];

                if (isOpen(nr, nc) && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[nr][nc] = current;
                    q.push({nr, nc});
                }
            }
        }

        auto end = high_resolution_clock::now();
        double timeTaken = duration<double, milli>(end - begin).count();
        bool found = !path.empty();
        int pathLength = found ? (int)path.size() - 1 : -1;
        vector<string> output = prepareOutputMaze(visited, path);

        return {found, pathLength, nodesVisited, timeTaken, output};
    }

    Result solveUsingDFS() {
        auto begin = high_resolution_clock::now();

        vector<vector<bool>> visited(rows, vector<bool>(cols, false));
        vector<vector<Cell>> parent(rows, vector<Cell>(cols, {-1, -1}));
        stack<Cell> st;
        vector<Cell> path;
        int nodesVisited = 0;

        int dr[] = {-1, 0, 1, 0};
        int dc[] = {0, 1, 0, -1};

        st.push(start);
        visited[start.row][start.col] = true;

        while (!st.empty()) {
            Cell current = st.top();
            st.pop();
            nodesVisited++;

            if (current.row == finish.row && current.col == finish.col) {
                path = buildPath(parent, finish);
                break;
            }

            for (int i = 0; i < 4; i++) {
                int nr = current.row + dr[i];
                int nc = current.col + dc[i];

                if (isOpen(nr, nc) && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[nr][nc] = current;
                    st.push({nr, nc});
                }
            }
        }

        auto end = high_resolution_clock::now();
        double timeTaken = duration<double, milli>(end - begin).count();
        bool found = !path.empty();
        int pathLength = found ? (int)path.size() - 1 : -1;
        vector<string> output = prepareOutputMaze(visited, path);

        return {found, pathLength, nodesVisited, timeTaken, output};
    }

    void displayResult(const string& algorithmName, const Result& result) {
        cout << "\n" << algorithmName << " Result:\n";

        if (result.found) {
            cout << "Path found\n";
            cout << "Path length: " << result.pathLength << " steps\n";
        } else {
            cout << "Path not found\n";
        }

        cout << "Nodes visited: " << result.nodesVisited << '\n';
        cout << "Time taken: " << result.timeTaken << " ms\n";
        cout << "Legend: S = Start, E = End, # = Wall, v = Visited, * = Final Path\n";
        displayMaze(result.outputMaze);
    }

    void compareBFSAndDFS() {
        Result bfs = solveUsingBFS();
        Result dfs = solveUsingDFS();

        cout << "\nBFS vs DFS Comparison:\n\n";
        cout << "Algorithm\tPath\tPath Length\tNodes Visited\tTime(ms)\n";
        cout << "BFS\t\t" << (bfs.found ? "Yes" : "No") << "\t";
        cout << bfs.pathLength << "\t\t" << bfs.nodesVisited << "\t\t" << bfs.timeTaken << '\n';
        cout << "DFS\t\t" << (dfs.found ? "Yes" : "No") << "\t";
        cout << dfs.pathLength << "\t\t" << dfs.nodesVisited << "\t\t" << dfs.timeTaken << '\n';

        cout << "\nTime Complexity:\n";
        cout << "BFS: O(R x C), where R is rows and C is columns\n";
        cout << "DFS: O(R x C), where R is rows and C is columns\n";
        cout << "BFS uses a queue and gives the shortest path in an unweighted maze.\n";
        cout << "DFS uses a stack and may find a path, but not always the shortest path.\n";
    }
};

int main() {
    MazeSolver solver;
    solver.loadDefaultMaze();

    int choice;

    do {
        cout << "\n========== Maze Solver ==========\n";
        cout << "1. Load default maze\n";
        cout << "2. Enter custom maze\n";
        cout << "3. Generate random maze\n";
        cout << "4. Display original maze\n";
        cout << "5. Solve using BFS\n";
        cout << "6. Solve using DFS\n";
        cout << "7. Compare BFS and DFS\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";

        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1:
                solver.loadDefaultMaze();
                cout << "Default maze loaded successfully.\n";
                break;

            case 2:
                if (solver.loadCustomMaze()) {
                    cout << "Custom maze loaded successfully.\n";
                }
                break;

            case 3:
                solver.generateRandomMaze();
                break;

            case 4:
                solver.displayOriginalMaze();
                break;

            case 5: {
                Result result = solver.solveUsingBFS();
                solver.displayResult("BFS", result);
                break;
            }

            case 6: {
                Result result = solver.solveUsingDFS();
                solver.displayResult("DFS", result);
                break;
            }

            case 7:
                solver.compareBFSAndDFS();
                break;

            case 0:
                cout << "Program ended.\n";
                break;

            default:
                cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 0);

    return 0;
}
