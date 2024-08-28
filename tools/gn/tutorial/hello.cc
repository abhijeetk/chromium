// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/tutorial/hello.h"
#include "hello_world.h"

#include <stdio.h>

void Hello(const char* who) {
  printf("Hello, %s.\nCalling hello world : ", who);
  hellow_world();
}

#if defined(TWO_PEOPLE)
void Hello(const char* one, const char* two) {
  printf("Hello, %s and %s.\n", one, two);
}
#endif
