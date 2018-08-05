# Taken from https://gist.github.com/bastman/5b57ddb3c11942094f8d0a97d461b430

# remove docker images
docker rmi $(docker images | grep "none" | awk '/ / { print $3 }')

# remove docker containers
docker rm $(docker ps -qa --no-trunc --filter "status=exited")
