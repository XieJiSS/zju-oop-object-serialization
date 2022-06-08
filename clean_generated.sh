#!/bin/bash

clean_generated_files() {
  rm -rf ./result/*.bin
  rm -rf ./result/*.xml
}

cd "$(dirname "$0")" && clean_generated_files