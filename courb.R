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
dataRaw <- read.csv(filename, header=T, sep = ",")

data <- ddply(
  dataRaw,
  c("steps", "height", "width", "nbCells", "fpOpByStep"),
  summarise,
  timeInµSec_min=min(timeInµSec),
  timeInµSec_mean=mean(timeInµSec),
  timeInµSec_max=max(timeInµSec),
  gigaflops_min=min(gigaflops),
  gigaflops_mean=mean(gigaflops),
  gigaflops_max=max(gigaflops),
  cellByS_min=min(cellByS),
  cellByS_mean=mean(cellByS),
  cellByS_max=max(cellByS),
  interactions=n(),
)

SIZE <- paste(data$width, data$height, sep="_")
data <- cbind(data, SIZE)

# GFlop/s
g <- ggplot(data, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs("Perfs Seq", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(data, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs("Time evolution Seq", x="nbCells", y="time(µ sec)")
plot(g)

filename <- "csv/mpi.csv"
data <- read.csv(filename, header=T, sep = ",")
