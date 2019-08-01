# unix-io-exercise

```sh
pushd dockerfiles/ex-build-env
docker build . -t ex-build-env
popd

pushd dockerfiles/ex-dev-server
docker build . -t ex-dev-server
popd

docker run -d --rm -p 127.0.0.1:2222:22 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined ex-dev-server
ssh root@localhost -p 2222
```
