FROM gitpod/workspace-full

USER root
# add your tools here
RUN apt-get update && apt-get install -y \
  imagemagick
