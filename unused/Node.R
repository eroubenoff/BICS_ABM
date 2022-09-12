setClass("Node", 
         slots = c(
           id = "numeric", 
           age = "numeric",
           hhsize = "numeric",
           num_cc = "numeric",
           num_cc_nonhh = "numeric",
           ethnicity = "character",
           disease_status = "character",
           remaining_days_sick = "numeric"
         )
) -> Node

setGeneric("set", function(Node, ...) standardGeneric("set"))
setGeneric("getID", function(.Object) standardGeneric("getID"))
setGeneric("disease_status", function(Node) standardGeneric("disease_status"))
setGeneric("disease_status<-", function(Node, value) standardGeneric("disease_status<-"))
setGeneric("remaining_days_sick", function(Node) standardGeneric("remaining_days_sick"))
setGeneric("remaining_days_sick<-", function(Node, value) standardGeneric("remaining_days_sick<-"))
setGeneric("set_sick", function(Node, remaining_days_sick) standardGeneric("set_sick"))


setMethod("set", "Node", function(Node, ...) {
  arguments <- list(...)
  
  for (a in names(arguments)) {
    attr(Node, a) <- arguments[[a]]
  }
  
  return(Node)
})

setMethod("getID", "Node", function(.Object) .Object@id)

setMethod("diseaseStatus", "Node", function(Node) Node@disease_status)

setMethod("diseaseStatus<-", "Node", function(Node, value) {
  Node@disease_status <- value; Node
})

setMethod("remainingDaysSick", "Node", function(Node) Node@remaining_days_sick)

setMethod("remainingDaysSick<-", "Node", function(Node, value) {
  Node@remaining_days_sick <- value; Node
})

setMethod("setSick", "Node", function(Node, remaining_days_sick) {
  diseaseStatus(Node) <- "I"
  remainingDaysSick(Node) <- remaining_days_sick
  Node
})


test_Node <- Node(id = 1, age = 26, hhsize = 2, num_cc = 5, num_cc_nonhh = 4,
                  ethnicity = "white", disease_status = "S", 
                  remaining_days_sick= NA_integer_)
set(test_Node, hhsize = 0)
set_sick(test_Node, 10)
disease_status(test_Node)
