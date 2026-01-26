# Docker support for gAgent

## Build image

```bash
docker build -t gagent:latest .
```

## Run container

```bash
docker run -it --rm gagent:latest
```

## Development container

```bash
docker run -it --rm -v $(pwd):/workspace gagent:latest bash
```
