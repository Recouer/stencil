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

dataSeq = data

# GFlop/s
g <- ggplot(data, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs Seq", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(data, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution Seq", x="nbCells", y="time(µ sec)")
plot(g)

######################################

filename <- "csv/omp.csv"
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

dataOmp = data

# GFlop/s
g <- ggplot(data, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs mpi pure", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(data, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution mpi pure", x="nbCells", y="time(µ sec)")
plot(g)

######################################

filename <- "csv/mpiPure.csv"
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

dataMpiPure = data

# GFlop/s
g <- ggplot(data, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs mpi pure", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(data, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution mpi pure", x="nbCells", y="time(µ sec)")
plot(g)

######################################

filename <- "csv/mpiOmp.csv"
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

dataMpiOmp <- data

# GFlop/s
g <- ggplot(data, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs mpi+omp", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(data, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution mpi+omp", x="nbCells", y="time(µ sec)")
plot(g)

#================================

dataSeq["name"] = "seq"
dataOmp["name"] = "omp"
dataMpiPure["name"] = "MpiPure"
dataMpiOmp["name"] = "MpiOmp"

dataGlob <- rbind(dataSeq, dataOmp, dataMpiPure, dataMpiOmp)

dataGlob$gigaflops_min = dataGlob$gigaflops_min * 1000
dataGlob$gigaflops_mean = dataGlob$gigaflops_mean * 1000
dataGlob$gigaflops_max = dataGlob$gigaflops_max * 1000

# GFlop/s
g <- ggplot(dataGlob, aes(x=nbCells, y=gigaflops_mean, color=name))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs global", x="nbCells", y="MFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(dataGlob, aes(x=nbCells, y=timeInµSec_mean, color=name))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution global", x="nbCells", y="time(µ sec)")
plot(g)

g <- ggplot(dataGlob)
g <- g + geom_col(aes(x=name, y=gigaflops_mean), position=position_dodge2(reverse=TRUE, padding=0), width=0.8)
g <- g + scale_y_continuous(trans='log2')
g <- g + scale_x_discrete(labels=name)
g <- g + labs(title="Global perfs", x="Variantes", y="MFlop/s")
plot(g)

g <- ggplot(dataGlob)
g <- g + geom_col(aes(x=name, y=timeInµSec_mean), position=position_dodge2(reverse=TRUE, padding=0), width=0.8)
g <- g + scale_y_continuous(limits = c(0, NA)) ## force the y axis to start at 0
g <- g + scale_x_discrete(labels=name)
g <- g + labs(title="Global exec time", x="Variantes", y="time(µ sec)")
plot(g)

