#!/bin/bash

ollama serve &
sleep 10
$*
pkill ollama
