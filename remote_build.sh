#!/bin/bash

PROJECT="sparkprime-gcp"
INSTANCE_NAME="grit-build"
ZONE="us-central1-b"
ENDPOINT="$INSTANCE_NAME.gritengine.com"
PORT="3389"
RES="1920x1080"
USERNAME="sparkprime"

gcloud --project="$PROJECT" compute instances start "$INSTANCE_NAME" --zone="$ZONE"
echo 'Waiting for RDP to come up...'
while ! echo exit | nc "$ENDPOINT" "$PORT"; do sleep 1; done
echo 'RDP up!'
echo 'Use shutdown /s /t 0 from the Windows console to shut down.'
xfreerdp /u:"$USERNAME" -grab-keyboard /size:"$RES" /p:"$(cat ~/.grit-build-pass)" /v:"${ENDPOINT}:${PORT}"
echo 'Waiting for instance to stop...'
while true; do
    STATUS="$(gcloud --project="$PROJECT" compute instances describe --zone="$ZONE" "$INSTANCE_NAME" --format=json | jsonnet -S -e '(import "/dev/stdin").status')"
    test "$STATUS" == 'TERMINATED' && break
    echo "Status: $STATUS"
    sleep 5
done

