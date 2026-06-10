package gee

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"reflect"
	"testing"
)

func newTestRouter() *router {
	r := newRouter()
	r.addRoute("GET", "/", nil)
	r.addRoute("GET", "/hello/:name", nil)
	r.addRoute("GET", "/hello/b/c", nil)
	r.addRoute("GET", "/hi/:name", nil)
	r.addRoute("GET", "/assets/*filepath", nil)
	return r
}

func TestParsePattern(t *testing.T) {
	ok := reflect.DeepEqual(parsePattern("/p/:name"), []string{"p", ":name"})
	ok = ok && reflect.DeepEqual(parsePattern("/p/*"), []string{"p", "*"})
	ok = ok && reflect.DeepEqual(parsePattern("/p/*name/*"), []string{"p", "*name"})
	if !ok {
		t.Fatal("test parsePattern failed")
	}
}

func TestGetRoute(t *testing.T) {
	r := newTestRouter()
	n, ps := r.getRoute("GET", "/hello/geektutu")

	if n == nil {
		t.Fatal("nil shouldn't be returned")
	}

	if n.pattern != "/hello/:name" {
		t.Fatal("should match /hello/:name")
	}

	if ps["name"] != "geektutu" {
		t.Fatal("name should be equal to 'geektutu'")
	}

	fmt.Printf("matched path: %s, params['name']: %s\n", n.pattern, ps["name"])
}

func TestGetRoute2(t *testing.T) {
	r := newTestRouter()
	n1, ps1 := r.getRoute("GET", "/assets/file1.txt")
	ok1 := n1.pattern == "/assets/*filepath" && ps1["filepath"] == "file1.txt"
	if !ok1 {
		t.Fatal("pattern shoule be /assets/*filepath & filepath shoule be file1.txt")
	}

	n2, ps2 := r.getRoute("GET", "/assets/css/test.css")
	ok2 := n2.pattern == "/assets/*filepath" && ps2["filepath"] == "css/test.css"
	if !ok2 {
		t.Fatal("pattern shoule be /assets/*filepath & filepath shoule be css/test.css")
	}

}

func TestGetRoutes(t *testing.T) {
	r := newTestRouter()
	nodes := r.getRoutes("GET")
	for i, n := range nodes {
		fmt.Println(i+1, n)
	}

	if len(nodes) != 5 {
		t.Fatal("the number of routes shoule be 4")
	}
}
func TestHandleRouteParams(t *testing.T) {
	r := newRouter()

	var helloName string
	r.addRoute("GET", "/hello/:name", func(c *Context) {
		helloName = c.Param("name")
		c.String(http.StatusOK, "ok")
	})

	w1 := httptest.NewRecorder()
	req1, _ := http.NewRequest("GET", "/hello/geektutu", nil)
	r.handle(newContext(w1, req1))
	if helloName != "geektutu" {
		t.Fatalf("expected helloName to be geektutu, got %s", helloName)
	}
	if w1.Code != http.StatusOK {
		t.Fatalf("expected status %d, got %d", http.StatusOK, w1.Code)
	}

	var filePath string
	r.addRoute("GET", "/assets/*filepath", func(c *Context) {
		filePath = c.Param("filepath")
		c.String(http.StatusOK, "ok")
	})

	w2 := httptest.NewRecorder()
	req2, _ := http.NewRequest("GET", "/assets/css/test.css", nil)
	r.handle(newContext(w2, req2))
	if filePath != "css/test.css" {
		t.Fatalf("expected filePath to be css/test.css, got %s", filePath)
	}
	if w2.Code != http.StatusOK {
		t.Fatalf("expected status %d, got %d", http.StatusOK, w2.Code)
	}
}
