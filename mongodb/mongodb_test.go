package mongodb

import (
	"testing"
	"github.com/coldume/pulse"
	"fmt"
)

var _ = fmt.Println

var _ = pulse.User{}

func TestM(t *testing.T) {
	conn := Open("mongodb://127.0.0.1/", "test");
	conn.CreateIndex()
	//err := conn.Register(
	//	&pulse.User{"coldume@gmail.com", "kikowen", "Xianghan Wang"},
	//	&pulse.Keys{"adfaf", "adsfadf", "4572405", "t2g", "2g", "2t", "rg", "24t"},
	//)
	//if err != nil {
	//	t.Fatal(err)
	//}

	//id, err := conn.Login("coldume@gmail.com", "kikowen")
	//if err != nil {
	//	t.Fatal(err)
	//}
	//println(id)

	//if err := conn.Unregister("5708068499976a3fa66aba41"); err != nil {
	//	t.Fatal(err)
	//}

	 // user, err := conn.GetUser("5708412799976a461b79a731")
	 // if err != nil {
	 //    t.Fatal(err)
	 // }
	 // fmt.Printf("%#v\n", user)

	//u := &pulse.User{
	//	Email: "hehe@haha.com",
	//	Name: "hhhh",
	//	Password: "wenkiko",
	//}
	//if err := conn.UpdateUser("5708412799976a461b79a731", u); err != nil {
	//	t.Fatal(err)
	//}

	//  keys, err := conn.GetKeys("5708412799976a461b79a731")
	//  if err != nil {
	//     t.Fatal(err)
	//  }
	//  fmt.Printf("%#v\n", keys)

	if err := conn.ReplaceKeys("5708412799976a461b79a731", &pulse.Keys{Iciba: "lala", MWSchoolDictionary: "xxxx"}); err != nil {
		t.Fatal(err)
	}
	conn.Close()
	Teardown()
}
