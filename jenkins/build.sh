#/bin/bash
set -x
set -e
docker build -t scale/openphysyn --target base-dependencies .
docker run -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd):/OpenPhySyn scale/openphysyn bash -c "/OpenPhySyn/jenkins/install.sh"