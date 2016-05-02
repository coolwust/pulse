package bson

import (
	"bytes"
	"bytes"
)

/*
bson:"-"
bson:"name"
bson:",omitempty"
bson:"name,omitempty"
*/

func Marshal(v interface{}) ([]byte, error) {
	var f func(rv reflect.Value) []byte
	f = func(rv reflect.Value) []byte {
		var buf bytes.Buffer
	}
}

//func Unmarshal(data []byte, v interface) error {
//}
