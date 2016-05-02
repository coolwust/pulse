package session

import (
	"crypto/rand"
	"fmt"
	"time"
)

type Session struct {
	UUID string                     `json:"uuid"`
	Duration int64                  `json:"duration"`
	Expires  int64                  `json:"expires"`
	Data     map[string]interface{} `json:"data"`
	Activity struct {
		Device   string             `json:"device"`
		Location string             `json:"location"`
		End      int64              `json:"end"`
	}                               `json:"activity"`
}

func NewSession(duration int64, device, location string) (*Session, error) {
	uuid, err := UUID()
	if err != nil {
		return nil, err
	}
	now := time.Now().Unix()
	s := &Session{
		UUID: uuid,
		Duration: duration,
		Expires: now + duration,
		Data: make(map[string]interface{}),
	}
	s.Activity.Device = device
	s.Activity.Location = location
	s.Activity.End = now
	return s, nil
}

func (s *Session) Delete(k string) {
	delete(s.Data, k)
}

func (s *Session) SetString(k, v string) {
	s.Data[k] = v
}

func (s *Session) String(k string) string {
	return s.Data[k].(string)
}

func (s *Session) SetFloat64(k string, v float64) {
	s.Data[k] = v
}

func (s *Session) Float64(k string) float64 {
	return s.Data[k].(float64)
}

func UUID() (string, error) {
	b := make([]byte, 16)
	if _, err := rand.Read(b); err != nil {
		return "", err
	}
	return fmt.Sprintf("%x-%x-%x-%x-%x", b[:4], b[4:6], b[6:8], b[8:10], b[10:]), nil
}
