# Urban Engine Flow 

---

## System Architecture

```
                    +------------------------------------+
                    |         User Input Query           |
                    | "Find bottleneck from Hall 6..."   |
                    +-----------------+------------------+
                                      |
                                      v
                    +-----------------+------------------+
                    |          Gemini LLM                |
                    | (Picks tool & extracts parameters) |
                    +-----------------+------------------+
                                      |
                                      v
                    +-----------------+------------------+
                    |        Python Pipeline             |
                    | (Resolves locations & maps graph)  |
                    +-----------------+------------------+
                                      |
                         (JSON Input over Stdin)
                                      v
                    +-----------------+------------------+
                    |        C++ Graph Engine            |
                    | (Runs Dijkstra / Dinic's / Min-Cut)|
                    +-----------------+------------------+
                                      |
                        (JSON Output over Stdout)
                                      v
                    +-----------------+------------------+
                    |          Gemini LLM                |
                    | (Translates raw output to English) |
                    +-----------------+------------------+
                                      |
                                      v
                    +-----------------+------------------+
                    |         Final Agent Response       |
                    +------------------------------------+
```

The system operates in a 4-step pipeline:
1. **Tool Identification**: Gemini parses user queries in plain English (e.g., *"What is the quickest way from Hall 6 to Hall 12?"*) and routes them to the correct tool.
2. **Network Remapping**: Python queries real OpenStreetMap data via `OSMnx`, resolves landmarks to coordinate nodes, and builds a JSON graph payload.
3. **High-Performance Execution**: The compiled C++ binary processes the graph, executing graph algorithms natively.
4. **Natural Language Explanation**: Gemini explains the result (e.g. converting paths to road names or meters to kilometers).

---

## Features

- **Shortest/Fastest Path (Dijkstra)**: Finds the quickest route between landmarks based on actual road segment lengths.
- **Maximum Sustainable Flow (Dinic's)**: Computes the peak vehicle capacity (vehicles/hour) the network can handle between two locations.
- **Traffic Bottleneck Localization (Min-Cut)**: Pinpoints the specific streets causing congestion/bottlenecks limiting overall flow.
- **Bridge Locator**: Finds and lists all bridges or elevated roadways in the selected network graph.
- **Dynamic Landmarks**: Easily customizable coordinates for target areas.

---

## Prerequisites

- **C++ Compiler**: A compiler supporting C++17 (e.g., `g++` / clang).
- **Python**: Python 3.9 or higher.
- **Gemini API Key**: A developer key from [Google AI Studio](https://aistudio.google.com/).

---

## Setup & Installation

### 1. Compile the C++ Graph Engine
The Python bridge requires a compiled binary named `engine` to exist in the root folder. Compile it using:
```bash
g++ -O2 -std=c++17 engine.cpp -o engine
```

### 2. Set Up the Python Virtual Environment
Create a virtual environment and install the required dependencies:
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install osmnx scikit-learn google-genai
```
*(Note: `scikit-learn` is required for spatial KDTree searches in `OSMnx` on unprojected graphs).*

### 3. Set Your Gemini API Key
Expose your Gemini key to the terminal:

* **macOS / Linux**:
  ```bash
  export GEMINI_API_KEY="your_api_key"
  ```
* **Windows (PowerShell)**:
  ```powershell
  $env:GEMINI_API_KEY="your_api_key"
  ```

---

## How to Run

### Test the Python-C++ Bridge
Sanity-check the integration with pre-packaged static graph test cases:
```bash
python3 engine.py
```

### Run the Interactive Agent
Launch the natural language terminal loop:
```bash
python3 agent.py
```
**Example Queries to Try:**
- `"What is the shortest path from Hall 6 to Gym?"`
- `"What is the maximum traffic capacity between Gym and Main Gate?"`
- `"Where is the traffic bottleneck from Hall 12 to Hall 6?"`
- `"Are there any bridges nearby?"`
- Type `quit` or `exit` to close the loop.

---

## Customizing the Map Area & Landmarks
To modify the network focus area or landmarks, edit the variables at the top of [agent.py](agent.py):

* **`CENTER_POINT`**: Latitude/longitude tuple around which the map is downloaded.
* **`DOWNLOAD_RADIUS_M`**: Distance (in meters) to retrieve graph edges.
* **`LANDMARKS`**: Dictionary mapping lowercase landmark names to GPS coordinates.
