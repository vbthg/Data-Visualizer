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
        }}


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
        }}
    };
}
