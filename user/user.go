

func (conn *Conn) Register(user *pulse.User, keys *pulse.Keys) error {
	u := C.CString(string(MustMarshal(user)))
	defer C.free(unsafe.Pointer(u))
	k := C.CString(string(MustMarshal(keys)))
	defer C.free(unsafe.Pointer(k))
	err := new(C.bson_error_t)
	C.mongodb_register(conn.db, u, k, err)
	if err.code == 11000 {
		return ErrDuplicateEmail
	}
	if err.code != 0 {
		return errors.New("AddUser: " + C.GoString(&err.message[0]));
	}
	return nil
}

func (conn *Conn) Unregister(id string) error {
	i := C.CString(id)
	defer C.free(unsafe.Pointer(i))
	err := new(C.bson_error_t)
	if C.mongodb_unregister(conn.db, i, err) != 0 {
		return errors.New("Unregister: " + C.GoString(&err.message[0]))
	}
	return nil
}

func (conn *Conn) Login(email, passwd string) (id string, _ error) {
	oidstr := C.malloc(25)
	if oidstr == nil {
		return "", errors.New("Login: out of memory, malloc failed")
	}
	defer C.free(oidstr)
	e := C.CString(email)
	defer C.free(unsafe.Pointer(e))
	p := C.CString(passwd)
	defer C.free(unsafe.Pointer(p))
	err := new(C.bson_error_t)
	if C.mongodb_login(conn.db, e, p, (*C.char)(oidstr), err) != 0 {
		return "", errors.New("Login: " + C.GoString(&err.message[0]))
	}
	return C.GoString((*C.char)(oidstr)), nil
}

func (conn *Conn) GetUser(id string) (user *pulse.User, _ error) {
	i := C.CString(id)
	defer C.free(unsafe.Pointer(i))
	err := new(C.bson_error_t)
	u := unsafe.Pointer(nil)
	defer C.bson_free(u)
	l := (C.size_t)(0)
	if C.mongodb_get_user(conn.db, i, &u, &l, err) != 0 {
		return nil, errors.New("GetUser: " + C.GoString(&err.message[0]))
	}
	if u == nil {
		return nil, ErrNoSuchUser
	}
	user = new(pulse.User)
	e := json.Unmarshal(C.GoBytes(u, (C.int)(l)), user)
	return user, e
}

func (conn *Conn) UpdateUser(id string, user *pulse.User) error {
	i := C.CString(id)
	defer C.free(unsafe.Pointer(i))
	u := C.CString(string(MustMarshal(user)))
	defer C.free(unsafe.Pointer(u))
	err := new(C.bson_error_t)
	if C.mongodb_update_user(conn.db, i, u, err) != 0 {
		return errors.New("UpdateUser: " + C.GoString(&err.message[0]))
	}
	return nil
}

func (conn *Conn) GetKeys(id string) (keys *pulse.Keys, err error) {
	i := C.CString(id)
	defer C.free(unsafe.Pointer(i))
	k := unsafe.Pointer(nil)
	l := (C.size_t)(0)
	defer C.bson_free(k)
	e := new(C.bson_error_t)
	if C.mongodb_get_keys(conn.db, i, &k, &l, e) != 0 {
		return nil, errors.New("GetKeys: " + C.GoString(&e.message[0]))
	}
	keys = new(pulse.Keys)
	err = json.Unmarshal(C.GoBytes(k, (C.int)(l)), keys)
	return keys, err
}

func (conn *Conn) ReplaceKeys(id string, keys *pulse.Keys) error {
	i := C.CString(id)
	defer C.free(unsafe.Pointer(i))
	k := C.CString(string(MustMarshal(keys)))
	defer C.free(unsafe.Pointer(k))
	err := new(C.bson_error_t)
	if C.mongodb_replace_keys(conn.db, i, k, err) != 0 {
		return errors.New("ReplaceKeys: " + C.GoString(&err.message[0]))
	}
	return nil
}

func (conn *Conn) GetWordJson(id string) (string, error) {
	i := C.CString(id)
	defer C.free(unsafe.Pointer(i))
	w := (*C.char)(nil)
	defer C.bson_free(w)
	err := new(C.bson_error_t)
	if C.mongodb_get_word(conn.db, i, &w, nil, err) != 0 {
		return "", errors.New("GetWordJson: " + C.GoString(&err.message[0]))
	}
	if w == nil {
		return "", ErrWordNotFound
	}
	return C.GoString(w), nil
}
