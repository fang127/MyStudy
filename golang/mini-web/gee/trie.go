package gee

import (
	"fmt"
	"strings"
)

type node struct {
	pattern  string  // 待匹配的路由，只有在叶子节点才会有值，例如 /p/:lang/doc，可以使用n.pattern == ""来判断路由规则是否匹配成功
	part     string  // 路由中的一部分，例如 :lang
	children []*node // 子节点，例如 [doc, tutorial, intro]
	isWild   bool    // 是否精确匹配，part 含有 : 或 * 时为true
}

// 第一个匹配成功的节点
func (n *node) String() string {
	return fmt.Sprintf("node{pattern=%s, part=%s, isWild=%t}", n.pattern, n.part, n.isWild)
}

// 插入节点，pattern是完整的路由，例如 /p/:lang/doc，parts是切分后的路由，例如 [p, :lang, doc]，height是当前树的高度
func (n *node) insert(pattern string, parts []string, height int) {
	// 递归出口，说明已经到达树的底部了
	if len(parts) == height {
		n.pattern = pattern
		return
	}

	// 获取当前节点的part，例如p、:lang、doc
	// 如果没有匹配到，就创建一个新的节点，并且添加到当前节点的children中
	// 如果匹配到了，就继续往下递归，直到找到叶子节点
	part := parts[height]
	child := n.matchChild(part)
	if child == nil {
		child = &node{part: part, isWild: part[0] == ':' || part[0] == '*'}
		n.children = append(n.children, child)
	}
	child.insert(pattern, parts, height+1)
}

// 查询结点，parts是切分后的路由，例如 [p, :lang, doc]，height是当前树的高度
func (n *node) search(parts []string, height int) *node {
	// 递归出口，说明已经到达树的底部了，或者当前节点的part是*，说明已经匹配成功了
	if len(parts) == height || strings.HasPrefix(n.part, "*") {
		if n.pattern == "" {
			return nil
		}
		return n
	}

	// 获取当前节点的part，例如p、:lang、doc
	// 如果没有匹配到，就返回nil
	// 如果匹配到了，就继续往下递归
	part := parts[height]
	children := n.matchChildren(part)
	for _, child := range children {
		res := child.search(parts, height+1)
		if res != nil {
			return res
		}
	}
	return nil
}

// 遍历节点，找到所有的叶子节点，添加到list中
func (n *node) travel(list *([]*node)) {
	if n.pattern != "" {
		*list = append(*list, n)
	}
	for _, child := range n.children {
		child.travel(list)
	}
}

// 匹配第一个成功的节点，用于插入
func (n *node) matchChild(part string) *node {
	for _, child := range n.children {
		if child.part == part || child.isWild {
			return child
		}
	}
	return nil
}

// 匹配所有成功的节点，用于查找
func (n *node) matchChildren(part string) []*node {
	nodes := make([]*node, 0)
	for _, child := range n.children {
		if child.part == part || child.isWild {
			nodes = append(nodes, child)
		}
	}
	return nodes
}
