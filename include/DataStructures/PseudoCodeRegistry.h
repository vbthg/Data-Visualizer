#pragma once
#include <map>
#include <vector>
#include <string>

namespace Resources
{
    // Cấu trúc: Map <Tên_Thuật_Toán_Hành_Động, Các_Dòng_Code>
    inline const std::map<std::string, std::vector<std::string>> PseudoCodeRegistry =
    {
        // --- AVL TREE ---
        { "avl_insert", {
            "insert(node, val)",
            "if node null",
            "    return new Node(val)",
            "if val < node.val",
            "    node.left = insert(node.left, val)",
            "else if val > node.val",
            "    node.right = insert(node.right, val)",
            "else return node",
            "update height(node)",
            "b = balance(node)",
            "if b > 1 and val < left.val",
            "    return rotateRight(node)",
            "if b < -1 and val > right.val",
            "    return rotateLeft(node)",
            "if b > 1 and val > left.val",
            "    left = rotateLeft(left),",
            "    return rotateRight(node)",
            "if b < -1 and val < right.val",
            "    right = rotateRight(right),",
            "    return rotateLeft(node)",
            "return node",
        }},

        { "avl_remove", {
            "remove(node, val)",
            "if node null then return null",
            "if val < node.val",
            "    node.left = remove(node.left, val)",
            "else if val > node.val",
            "    node.right = remove(node.right, val)",
            "else if node has <= 1 child",
            "    node = existing child (or null)",
            "else",
            "    temp = min(node.right)",
            "    node.val = temp.val",
            "    node.right = remove(node.right, temp.val)",
            "if node null",
            "    return null",
            "update height(node)",
            "b = balance(node)",
            "if b > 1 and bal(left) >= 0 return rotateRight(node)",
            "if b < -1 and bal(right) <= 0 return rotateLeft(node)",
            "if b > 1 and bal(left) < 0 left = rotateLeft(left), return rotateRight(node)",
            "if b < -1 and bal(right) > 0 right = rotateRight(right), return rotateLeft(node)",
            "return node",
        }},

        { "avl_search", {
            "search(root, val)",
            "curr = root",
            "while curr != null",
            "    if val == curr.val",
            "        return true",
            "    if val < curr.val",
            "        curr = curr.left",
            "    else",
            "        curr = curr.right",
            "return false",
        }},

        { "avl_clear", {
            "function clear(node)",
            "{",
            "    if(node == null) return",
            "    clear(node.left)",
            "    clear(node.right)",
            "    delete node",
            "}",
            "clear(root); root = null"
        }},

        // --- HEAP ---
        { "heap_insert", {
            "push(val)",
            "data.push_back(val)",
            "curr = last index",
            "while curr > 0",
            "    p = (curr - 1) / 2",
            "    if data[curr] better than data[p]",
            "        swap(data[curr], data[p])",
            "        curr = p",
            "    else",
            "        break",
        }},

        { "heap_extract", {
            "extract()",
            "res = data[0]",
            "data[0] = data.back()",
            "data.pop_back()",
            "curr = 0",
            "while hasLeftChild(curr)",
            "    target = getTargetChild(curr)",
            "    if data[target] better than data[curr]",
            "        swap(data[curr], data[target])",
            "        curr = target",
            "    else",
            "        break",
            "return res",
        }},

        { "heap_toggle", {
            "toggleHeap()",
            "isMaxHeap = !isMaxHeap",
            "for i = (size / 2) - 1 down to 0",
            "    heapifyDown(i)",
        }},

        // --- MST (KRUSKAL) ---
        { "mst_add_edge", {
            "addEdge(u, v, w)",
            "edges.push_back({u, v, w})",
        }},

        { "kruskal_mst", {
            "addEdge(u, v, w)",
            "edges.push_back({u, v, w})",
            ".",
            ".",
            "kruskal()",
            "DSU dsu(V)",
            "sort edges by weight",
            "for each edge (u, v, w)",
            "    if find(u) != find(v)",
            "        union(u, v)",
            "        MST.add({u, v, w})",
            "if all nodes connected then MST valid",
            "else MST invalid",
            ".",
            ".",
            "clear()",
            "edges.clear()",
            "MST.clear()",
            "dsu.reset()"
        }},

        { "mst_clear", {
            "clear()",
            "edges.clear()",
            "MST.clear()",
            "dsu.reset()"
        }},

        // --- SHORTEST PATH (DIJKSTRA) ---
        { "dijkstra_shortest_path", {
            "addEdge(u, v, w)",
            ".",
            "dijkstraTarget(src, target)",
            "dist[all] = INF",
            "dist[src] = 0",
            "PQ.push({0, src})",
            "while PQ not empty",
            "    (d, u) = PQ.pop()",
            "    if u == target",
            "        break",
            "    for each neighbor (v, w)",
            "        if dist[u] + w < dist[v]",
            "            dist[v] = dist[u] + w",
            "            parent[v] = u",
            "            PQ.push({dist[v], v})",
            "reconstructPath(target)",
            ".",
            "clear()"
        }},

        { "sp_add_edge", {
            "addEdge(u, v, w)",
            "adj[u].push_back({v, w})",
        }},

        { "sp_dijkstra_all", {
            "dijkstraAll(src)",
            "dist[all] = INF",
            "dist[src] = 0",
            "PQ.push({0, src})",
            "while PQ not empty",
            "    (d, u) = PQ.pop()",
            "    if d > dist[u]",
            "        continue",
            "    for each neighbor (v, w)",
            "        if dist[u] + w < dist[v]",
            "            dist[v] = dist[u] + w",
            "            PQ.push({dist[v], v})",
        }},

        { "sp_dijkstra_target", {
            "dijkstraTarget(src, target)",
            "dist[all] = INF",
            "dist[src] = 0",
            "PQ.push({0, src})",
            "while PQ not empty",
            "    (d, u) = PQ.pop()",
            "    if u == target",
            "        break",
            "    for each neighbor (v, w)",
            "        if dist[u] + w < dist[v]",
            "            dist[v] = dist[u] + w",
            "            parent[v] = u",
            "            PQ.push({dist[v], v})",
            "reconstructPath(target)",
        }},

        { "sp_clear", {
            "clear()",
            "adj.clear()",
            "dist.clear()",
            "parent.clear()",
        }},

        // --- SINGLY LINKED LIST ---
        { "sll_push_front", {
            "pushFront(val)",
            "newNode = new Node(val)",
            "newNode.next = head",
            "head = newNode",
        }},

        { "sll_push_back", {
            "pushBack(val)",
            "newNode = new Node(val)",
            "if head null",
            "    head = newNode",
            "else",
            "    curr = head",
            "    while curr.next != null",
            "        curr = curr.next",
            "    curr.next = newNode",
        }},

        { "sll_insert", {
            "insert(idx, val)",
            "newNode = new Node(val)",
            "if idx == 0",
            "    newNode.next = head",
            "    head = newNode",
            "else",
            "    curr = head, i = 0",
            "    while i < idx - 1 and curr != null",
            "        curr = curr.next, i++",
            "    newNode.next = curr.next",
            "    curr.next = newNode",
        }},

        { "sll_remove", {
            "remove(idx)",
            "if head null",
            "    return",
            "if idx == 0",
            "    temp = head",
            "    head = head.next",
            "    delete temp",
            "else",
            "    curr = head, i = 0",
            "    while i < idx - 1 and curr.next != null",
            "        curr = curr.next, i++",
            "    temp = curr.next",
            "    curr.next = temp.next",
            "    delete temp",
        }},

        { "sll_search", {
            "search(val)",
            "curr = head",
            "while curr != null",
            "    if curr.val == val",
            "        return true",
            "    curr = curr.next",
            "return false",
        }},

        { "sll_clear", {
            "clear()",
            "while head != null",
            "    temp = head",
            "    head = head.next",
            "    delete temp",
        }},

        // --- TRIE ---
        { "trie_insert", {
            "insert(word)",
            "curr = root",
            "for each char c in word",
            "    if c not in curr.children",
            "        curr.children[c] = new Node()",
            "    curr = curr.children[c]",
            "curr.isEndOfWord = true",
        }},

        { "trie_remove", {
            "remove(node, word, depth)",
            "if node null",
            "    return false",
            "if depth == word.length",
            "    node.isEndOfWord = false",
            "    return children empty",
            "c = word[depth]",
            "if remove(node.children[c], word, depth + 1)",
            "    delete node.children[c]",
            "    erase child c",
            "    return !isEndOfWord and children empty",
            "return false",
        }},

        { "trie_search", {
            "search(word)",
            "curr = root",
            "for each char c in word",
            "    if c not in curr.children",
            "        return false",
            "    curr = curr.children[c]",
            "return curr.isEndOfWord",
        }},

        { "trie_clear", {
            "clear()",
            "recursiveClear(root)",
            "root = new Node()",
        }}
    };
}
