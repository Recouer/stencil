library(plyr)
library(dplyr)
library(ggplot2)
#library(viridis)

# Clean
rm(list=ls(all=TRUE))
graphics.off()

# Set Env
currentScriptDir <- dirname(rstudioapi::getActiveDocumentContext()$path)
setwd(currentScriptDir)

# Load data
filename <- "csvPlafrim/seq.csv"
dataRawSeq <- read.csv(filename, header=T, sep = ",")
dataRawSeq["name"] = "seq"

filename <- "csvPlafrim/omp.csv"
dataRawOmp <- read.csv(filename, header=T, sep = ",")
dataRawOmp["name"] = "omp"

filename <- "csvPlafrim/omp_halos.csv"
dataRawOmpHalos <- read.csv(filename, header=T, sep = ",")
dataRawOmpHalos["name"] = "ompHalos"

#filename <- "csvPlafrim/mpiPure.csv"
#dataRawMpiPure <- read.csv(filename, header=T, sep = ",")
#dataRawMpiPure["name"] = "MpiPure"

#filename <- "csvPlafrim/mpiOmp.csv"
#dataRawMpiOmp <- read.csv(filename, header=T, sep = ",")
#dataRawMpiOmp["name"] = "MpiOmp"

#data <- rbind(dataRawSeq, dataRawOmp, dataRawMpiPure, dataRawMpiOmp)
dataRaw <- rbind(dataRawSeq, dataRawOmp)

data <- ddply(
  dataRaw,
  c("name","steps", "height", "width", "nbCells", "fpOpByStep"),
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

dataOmpHalos <- ddply(
  dataRawOmpHalos,
  c("name","steps", "height", "width", "tiledW", "tiledH", "nbCells", "fpOpByStep"),
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

###################################### Seq
filterData=data[data$name=="seq",]

# GFlop/s
g <- ggplot(filterData, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs Seq", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(filterData, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution Seq", x="nbCells", y="time(µ sec)")
plot(g)

###################################### Omp
filterData=data[data$name=="omp",]

# GFlop/s
g <- ggplot(filterData, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs mpi pure", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(filterData, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution mpi pure", x="nbCells", y="time(µ sec)")
plot(g)

###################################### MpiPure
filterData=data[data$name=="MpiPure",]

# GFlop/s
g <- ggplot(filterData, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs mpi pure", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(filterData, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution mpi pure", x="nbCells", y="time(µ sec)")
plot(g)

###################################### MpiOmp
filterData=data[data$name=="MpiOmp",]

# GFlop/s
g <- ggplot(filterData, aes(x=nbCells, y=gigaflops_mean))
g <- g + geom_ribbon(aes(ymin=gigaflops_min, ymax=gigaflops_max),alpha=0.2)
#g <- g + geom_errorbar(aes(ymin=gigaflops_min, ymax=gigaflops_max))
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Perfs mpi+omp", x="nbCells", y="GFlop/s")
plot(g)

# time(µ sec)
g <- ggplot(filterData, aes(x=nbCells, y=timeInµSec_mean))
g <- g + geom_ribbon(aes(ymin=timeInµSec_min, ymax=timeInµSec_max),alpha=0.2)
g <- g + geom_line()
g <- g + geom_point()
g <- g + labs(title="Time evolution mpi+omp", x="nbCells", y="time(µ sec)")
plot(g)

###################################### Compare
filterData=data[data$name %in% c("seq", "omp"),]
#filterData=data[data$name %in% c("seq", "omp", "MpiPure", "MpiOmp"),]

dataGlob <- filterData

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

