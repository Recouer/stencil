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

g <- ggplot(data, aes(x=nbCells, y=gigaflop_s))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs("Test Seq", x="nbCells", y="GFlop/s")
plot(g)

