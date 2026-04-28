#pragma once
#include <map>
#include <vector>
#include <string>

namespace Resources
{
    // Sử dụng 'inline' để tránh lỗi trùng lặp định nghĩa khi include nhiều nơi
    // Cấu trúc: Map <Tên_Thuật_Toán_Hành_Động, Các_Dòng_Code>
    inline const std::map<std::string, std::vector<std::string>> PseudoCodeRegistry =
    {
        // --- TRIE ALGORITHM ---
        { "trie_insert", {
            "node curr = root",
            "for each char c in word",
            "{",
            "    if c is not in curr.children",
            "    {",
            "        curr.children[c] = new node",
            "    }",
            "    curr = curr.children[c]",
            "}",
            "curr.isEndOfWord = true"
        }},

        { "trie_search", {
            "node curr = root",
            "for each char c in word",
            "{",
            "    if c is not in curr.children",
            "    {",
            "        return false",
            "    }",
            "    curr = curr.children[c]",
            "}",
            "return curr.isEndOfWord"
        }},

        { "trie_remove", {
            "function remove(node, word, depth)",
            "{",
            "    if node is null",
            "    {",
            "        return false",
            "    }",
            "    if depth == word.length",
            "    {",
            "        node.isEndOfWord = false",
            "        return node.children is empty",
            "    }",
            "    char c = word[depth]",
            "    if remove(node.children[c], word, depth + 1)",
            "    {",
            "        delete node.children[c]",
            "        return node.children is empty and not node.isEndOfWord",
            "    }",
            "    return false",
            "}"
        }},

        // --- AVL TREE (Ví dụ để bạn thấy sự nhất quán) ---
        { "avl_insert", {
            "node = recursive_insert(node, value)",
            "update_height(node)",
            "balance_factor = get_balance(node)",
            "if balance_factor > 1",
            "{",
            "    perform rotations",
            "}"
        }},

        { "avl_search", {
            "while(curr != null)",
            "    if(val == curr.data) return true",
            "    curr = (val < curr.data) ? curr.left : curr.right",
            "return false"
        }},
        { "avl_remove", {
            "node = recursive_remove(node, val)",
            "if node == null return null",
            "update_height(node)",
            "balance = get_balance(node)",
            "if balance requires rotations...",
            "    perform_rotations()",
            "return node"
        }},


        { "kruskal_mst", {
            "Sort all edges by weight",
            "DSU dsu(V)",
            "for each edge (u, v) in sorted_edges",
            "{",
            "    Highlight edge (u, v)",
            "    if(dsu.find(u) != dsu.find(v))",
            "    {",
            "        dsu.union(u, v)",
            "        Add edge (u, v) to MST",
            "    }",
            "    else",
            "    {",
            "        Discard edge (u, v)",
            "    }",
            "}",
            "Return MST"
        }},

        { "dijkstra_shortest_path", {
            "dist[all] = ∞, dist[source] = 0",
            "PQ.push({0, source})",
            "while PQ is not empty",
            "{",
            "    (d, u) = PQ.pop_min()",
            "    if d > dist[u] then continue",
            "    Mark u as Finalized",
            "    for each neighbor v of u",
            "    {",
            "        if dist[u] + w(u, v) < dist[v]",
            "        {",
            "            dist[v] = dist[u] + w(u, v)",
            "            parent[v] = u",
            "            PQ.push({dist[v], v})",
            "        }",
            "    }",
            "}"
        }},

        // Thêm vào trong PseudoCodeRegistry ở file tương ứng
        { "sll_push_front", {
            "newNode = new Node(value)",
            "newNode.next = head",
            "head = newNode",
            "update positions"
        }},

        { "sll_insert", {
            "newNode = new Node(value)",
            "curr = head, i = 0",
            "while i < index - 1",
            "{",
            "    curr = curr.next",
            "    i++",
            "}",
            "newNode.next = curr.next",
            "curr.next = newNode",
            "update positions"
        }},

        { "sll_remove", {
            "if index == 0 then head = head.next",
            "curr = head, i = 0",
            "while i < index - 1",
            "{",
            "    curr = curr.next",
            "    i++",
            "}",
            "temp = curr.next",
            "curr.next = temp.next",
            "delete temp",
            "update positions"
        }},


        { "heap_insert", {
            "data.push_back(value)",
            "curr = data.size() - 1",
            "while curr > 0 and data[curr] > data[parent]",
            "{",
            "    swap(data[curr], data[parent])",
            "    curr = parent",
            "}",
            "return"
        }},
        { "heap_extract", {
            "root = data[0]",
            "data[0] = data.back()",
            "data.pop_back()",
            "heapifyDown(0)",
            "while hasChild(curr)",
            "{",
            "    maxChild = getGreaterChild(curr)",
            "    if data[curr] < data[maxChild]",
            "        swap(data[curr], data[maxChild])",
            "    else break",
            "}"
        }}
    };
}
