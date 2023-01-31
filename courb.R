library(plyr)
library(dplyr)
library(ggplot2)
library(viridis)

# Clean
rm(list=ls(all=TRUE))
graphics.off()

# Set Env
currentScriptDir <- dirname(rstudioapi::getActiveDocumentContext()$path)
setwd(currentScriptDir)

filename <- "csv/seq.csv"
data <- read.csv(filename, header=T, sep = ",")

g <- ggplot(data, aes(x=1:nrow(data), y=gigaflops))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs("Perfs Seq", x="nrow", y="GFlop/s")
plot(g)

g <- ggplot(data, aes(x=1:nrow(data), y=timeInµSec))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs("Time evolution Seq", x="nrow", y="time(µ sec)")
plot(g)

SIZE <- paste(data$width, data$height, sep="_")
dataS <- cbind(data, SIZE)
g <- ggplot(dataS, aes(x=SIZE, y=gigaflops))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Test tiled", x="SIZE", y="gigaflops")
plot(g)

filename <- "csv/mpi.csv"
data <- read.csv(filename, header=T, sep = ",")

