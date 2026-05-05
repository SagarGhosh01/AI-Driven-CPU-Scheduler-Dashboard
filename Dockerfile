FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build natively for Linux
RUN mkdir build && cd build && \
    cmake .. && \
    make Server

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /app/build/Server .
COPY --from=build /app/templates ./templates
COPY --from=build /app/static ./static
COPY --from=build /app/json.hpp ./json.hpp

EXPOSE 5000
CMD ["./Server"]