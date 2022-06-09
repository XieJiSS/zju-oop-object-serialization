#!/bin/bash

clean_generated_files() {
  rm -rf ./result/*.bin
  rm -rf ./result/*.xml
  rm -rf ./result/*.b64
}

cd "$(dirname "$0")" && clean_generated_files
