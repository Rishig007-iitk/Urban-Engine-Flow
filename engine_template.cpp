#include <bits/stdc++.h>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;
// `rev` is the index, within the OTHER node's adjacency list, of
// this edge's paired reverse edge. This is what lets the flow
// algorithms "undo" flow they've pushed — a core idea in residual
// graphs. It is set up for you in buildGraph() below; you don't
// need to build this wiring yourself, just understand that it
// exists and how to use it (you'll need g.adj[u.to][u.rev] inside
// your flow algorithms).
struct Edge {
    int to;
    double weight;
    long long cap;
    long long flow = 0;
    int rev = -1;
};
struct Graph {
    int n;
    vector<vector<Edge>> adj;
    Graph(int vertices) : n(vertices), adj(vertices) {}
    void addEdge(int from, int to, double w, long long cap) {
        Edge a = {to, w, cap, 0, (int)adj[to].size()};
    Edge b = {from, 0, 0,   0, (int)adj[from].size()};

    adj[from].push_back(a);
    adj[to].push_back(b);
    }
};
Graph buildGraphForDijkstra(const json& input) {
    int n = input.at("num_nodes").get<int>();
    Graph g(n);
    for (auto& e : input.at("edges")) {
        int u = e.at("from").get<int>();
        int v = e.at("to").get<int>();
        double w = e.value("weight", 1.0);
        g.adj[u].push_back({v, w, 0, 0, -1});
        if (e.value("undirected", false)) {
            g.adj[v].push_back({u, w, 0, 0, -1});
        }
    }
    return g;
}
Graph buildGraph(const json& input) {
    int n = input.at("num_nodes").get<int>();
    Graph g(n);
    for (auto& e : input.at("edges")) {
        int u = e.at("from").get<int>();
        int v = e.at("to").get<int>();
        double w = e.value("weight", 1.0);
        long long cap = e.value("capacity", 0LL);
        g.addEdge(u, v, w, cap);
        if (e.value("undirected", false)) {
            g.addEdge(v, u, w, cap);
        }
    }
    return g;
}
json runDijkstra(const Graph& g, int source, int target) {
    json result;
    int n = g.n;
    vector<double> dist(n, numeric_limits<double>::infinity());
    vector<int> parent(n, -1);
    
    using pdi = pair<double, int>;
    priority_queue<pdi, vector<pdi>, greater<pdi>> pq;
    
    dist[source] = 0;
    pq.push({0, source});
    
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        
        if (d > dist[u]) continue;
        if (u == target) break;
        
        for (const auto& edge : g.adj[u]) {
            int v = edge.to;
            double w = edge.weight;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    
    if (dist[target] == numeric_limits<double>::infinity()) {
        result["found"] = false;
        return result;
    }
    
    vector<int> path;
    for (int curr = target; curr != -1; curr = parent[curr]) {
        path.push_back(curr);
    }
    reverse(path.begin(), path.end());
    
    result["found"] = true;
    result["distance"] = dist[target];
    result["path"] = path;
    return result;
}
json runDinic(Graph& g, int source, int target) {
    json result;
    long long flow = 0;
    int n = g.n;
    vector<int> level(n);
    
    auto bfs = [&]() {
        fill(level.begin(), level.end(), -1);
        level[source] = 0;
        queue<int> q;
        q.push(source);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const auto& edge : g.adj[u]) {
                if (edge.cap - edge.flow > 0 && level[edge.to] == -1) {
                    level[edge.to] = level[u] + 1;
                    q.push(edge.to);
                }
            }
        }
        return level[target] != -1;
    };
    
    vector<int> ptr(n);
    auto dfs = [&](auto& self, int u, long long pushed) -> long long {
        if (pushed == 0) return 0;
        if (u == target) return pushed;
        for (int& cid = ptr[u]; cid < (int)g.adj[u].size(); ++cid) {
            auto& edge = g.adj[u][cid];
            int v = edge.to;
            if (level[u] + 1 != level[v] || edge.cap - edge.flow == 0) continue;
            long long tr = self(self, v, min(pushed, edge.cap - edge.flow));
            if (tr == 0) continue;
            edge.flow += tr;
            g.adj[v][edge.rev].flow -= tr;
            return tr;
        }
        return 0;
    };
    
    while (bfs()) {
        fill(ptr.begin(), ptr.end(), 0);
        while (long long pushed = dfs(dfs, source, numeric_limits<long long>::max())) {
            flow += pushed;
        }
    }
    
    result["found"] = true;
    result["max_flow"] = flow;
    return result;
}
json runMinCut(Graph& g, int source, int target) {
    json result;
    
    json dinic_res = runDinic(g, source, target);
    
    int n = g.n;
    vector<bool> visited(n, false);
    queue<int> q;
    
    visited[source] = true;
    q.push(source);
    
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (const auto& edge : g.adj[u]) {
            if (!visited[edge.to] && edge.cap - edge.flow > 0) {
                visited[edge.to] = true;
                q.push(edge.to);
            }
        }
    }
    
    vector<json> cut_edges;
    vector<int> source_partition;
    for (int u = 0; u < n; ++u) {
        if (visited[u]) {
            source_partition.push_back(u);
            for (const auto& edge : g.adj[u]) {
                if (!visited[edge.to] && edge.cap > 0) {
                    cut_edges.push_back({
                        {"from", u},
                        {"to", edge.to},
                        {"capacity", edge.cap}
                    });
                }
            }
        }
    }
    
    result["found"] = true;
    result["max_flow"] = dinic_res["max_flow"];
    result["cut_edges"] = cut_edges;
    result["source_partition"] = source_partition;
    return result;
}

// ==================================================================
// DISPATCH — given, you shouldn't need to modify this
// ==================================================================
int main() {
    ostringstream ss;
    ss << cin.rdbuf();
    string input_str = ss.str();

    json input;
    try {
        input = json::parse(input_str);
    } catch (const exception& ex) {
        json err;
        err["error"] = string("invalid JSON input: ") + ex.what();
        std::cout << err.dump() << std::endl;
        return 1;
    }

    string algorithm = input.value("algorithm", "");
    int source = input.value("source", 0);
    int target = input.value("target", 0);

    json result;
    try {
        if (algorithm == "dijkstra") {
            Graph g = buildGraphForDijkstra(input);
            result = runDijkstra(g, source, target);
        } else if (algorithm == "dinic") {
            Graph g = buildGraph(input);
            result = runDinic(g, source, target);
        } else if (algorithm == "min_cut") {
            Graph g = buildGraph(input);
            result = runMinCut(g, source, target);
        } else {
            result["error"] = "unknown algorithm: " + algorithm;
            result["algorithm"] = algorithm;
            std::cout << result.dump() << std::endl;
            return 1;
        }
    } catch (const exception& ex) {
        result = json{};
        result["error"] = string("runtime error: ") + ex.what();
        std::cout << result.dump() << std::endl;
        return 1;
    }

    result["algorithm"] = algorithm;
    std::cout << result.dump() << std::endl;
    return 0;
}
