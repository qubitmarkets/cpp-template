// Copyright (c) 2025 Qubit Markets Pte. Ltd.
#include "qbuild/SigHandler.h"
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

int main(int argc, char **argv) {
  SigHandler::install();
  int rv = Catch::Session().run(argc, argv);
  return rv;
}