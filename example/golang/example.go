package main

/*
#cgo CFLAGS: -I${SRCDIR}/../../src
#cgo LDFLAGS: -L${SRCDIR}/../../build/src -lStatForge
#include "api/c.h"
#include <stdlib.h>
*/
import "C"

import (
	"fmt"
	"unsafe"
)

func cString(s string) *C.char {
	return C.CString(s)
}

func main() {
	engine := C.sf_create_engine()
	if engine == nil {
		fmt.Println("Error: could not create engine")
		return
	}
	defer C.sf_destroy_engine(engine)

	lifeNodeName := cString("life")
	defer C.free(unsafe.Pointer(lifeNodeName))

	C.sf_create_value_node(engine, lifeNodeName, 105.0)

	var nodeValue C.double
	err := C.sf_get_node_value(engine, lifeNodeName, &nodeValue)
	if err != C.SF_OK {
		fmt.Printf("Error: %s\n", C.GoString(C.sf_last_error()))
	} else {
		fmt.Printf("Val: %.0f\n", float64(nodeValue))
	}

	C.sf_create_value_node(engine, lifeNodeName, 100.0)
	fmt.Printf("Error: %s\n", C.GoString(C.sf_last_error()))
}
