package session

import (
	"testing"
	"regexp"
)

func TestUUID(t *testing.T) {
	uuid, err := UUID()
	if err != nil {
		t.Fatal(err)
	}
	re := regexp.MustCompile(`^[[:alnum:]]{8}(?:-[[:alnum:]]{4}){3}-[[:alnum:]]{12}$`)
	if !re.MatchString(uuid) {
		t.Fatalf("want an UUID in hexadecimal format, get %s", uuid)
	}
}
