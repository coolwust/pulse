package session

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base64"
	"strings"
	"crypto/subtle"
)

func Sign(unsigned, key string) string {
	h := hmac.New(sha256.New, []byte(key))
	h.Write([]byte(unsigned))
	t := strings.Map(func(r rune) rune {
		if r == '=' || r == '/' || r == '+' {
			return -1
		}
		return r
	}, base64.StdEncoding.EncodeToString(h.Sum(nil)))
	return unsigned + "." + strings.ToLower(t)
}

func Unsign(signed, key string) string {
	s := strings.Split(signed, ".")
	if len(s) != 2 || subtle.ConstantTimeCompare([]byte(signed), []byte(Sign(s[0], key))) != 1 {
		return ""
	}
	return s[0]
}
