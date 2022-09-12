setClass("Edge", 
         slots = c(
           Node1_id = "numeric",
           Node2_id = "numeric",
           Node1_disease_status = "character",
           Node2_disease_status = "character"
         )
) -> Edge

setMethod("initialize", "Edge", function(.Object, Node1, Node2) {
  .Object@Node1_id = Node1@id
  .Object@Node2_id = Node2@id
  .Object@Node1_disease_status = Node1@disease_status
  .Object@Node2_disease_status = Node2@disease_status
  return(.Object)
})
