#!/bin/bash

export CLOUDSDK_CORE_ACCOUNT="sparkprime@gmail.com"
export CLOUDSDK_CORE_PROJECT="sparkprime-gcp"

INSTANCE_NAME="grit-build"
ZONE="us-central1-b"

ENDPOINT="$INSTANCE_NAME.gritengine.com"
PORT="3389"
RES="1920x1080"
USERNAME="sparkprime"
PASSWORD="$(cat ~/.grit-build-pass)"

gcloud compute instances start "$INSTANCE_NAME" --zone="$ZONE"
echo 'Waiting for RDP to come up...'
while ! echo exit | nc "$ENDPOINT" "$PORT"; do sleep 1; done
echo 'RDP up!'
echo 'Use shutdown /s /t 0 from the Windows console to shut down.'
# This works for xfreerdp 1.0.2
xfreerdp  -u "$USERNAME" -K -g "$RES" -p "$PASSWORD" "ENDPOINT"
# This one is a later version (I think)
# xfreerdp /u:"$USERNAME" -grab-keyboard /size:"$RES" /p:"$PASSWORD" /v:"${ENDPOINT}:${PORT}"
echo 'Waiting for instance to stop...'
while true; do
    STATUS="$(gcloud compute instances describe --zone="$ZONE" "$INSTANCE_NAME" --format=json | jsonnet -S -e '(import "/dev/stdin").status')"
    test "$STATUS" == 'TERMINATED' && break
    echo "Status: $STATUS"
    sleep 5
done

