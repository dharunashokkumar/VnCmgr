#!/bin/bash
curl -s --unix-socket /var/lib/incus/unix.socket -H "Content-Type: application/json" -X POST -d '{
  "name": "api-test-ct",
  "source": {
    "type": "image",
    "mode": "pull",
    "server": "https://images.linuxcontainers.org",
    "protocol": "simplestreams",
    "alias": "alpine/3.19"
  }
}' http://localhost/1.0/instances
