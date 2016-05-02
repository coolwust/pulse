package session

import (
	"testing"
)

var signTests = []struct {
	unsigned string
	key      string
	want     string
}{
	{"hello", "world", "hello.ppp27xstfbwoprn4av6gpzqsnqgoi80kdwfccuahs"},
    {"foo", "bar", "foo.fhkziyqqvaulekkzpcnghmjzq0g88qphntxycpd0gfe"},
}

func TestSign(t *testing.T) {
	for i, tt := range signTests {
		if signed := Sign(tt.unsigned, tt.key); signed != tt.want {
			t.Errorf("%d: got %s, want %s\n", i, signed, tt.want)
		}
	}
}
var unsignTests = []struct {
	signed string
	key    string
	want   string
}{
	{"hello.ppp27xstfbwoprn4av6gpzqsnqgoi80kdwfccuahs", "world", "hello"},
    {"foo.fhkziyqqvaulekkzpcnghmjzq0g88qphntxycpd0gfe", "bar", "foo"},
    {"baz.qux", "quux", ""},
}

func TestUnsign(t *testing.T) {
	for i, tt := range unsignTests {
		if unsigned := Unsign(tt.signed, tt.key); unsigned != tt.want {
			t.Errorf("%d: got %s, want %s\n", i, unsigned, tt.want)
		}
	}
}
