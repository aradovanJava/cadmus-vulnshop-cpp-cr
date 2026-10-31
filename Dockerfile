# EXCERPT B, continued. Four findings in nine lines.
FROM ubuntu:latest
RUN apt-get update && apt-get install -y g++ make && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN make
EXPOSE 8080 9090
USER root
CMD ["./build/vulnshop", "config/vulnshop.ini"]
