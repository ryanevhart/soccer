(define (domain SOCCER)
	(:requirements :strips :equality)
	
	(:predicates 
  		(at ?x)
    	(connected ?s ?g))

	(:action move
		:parameters (?s ?g)
		:precondition (and (or(connected ?s ?g) (connected ?g ?s)) (AT ?s))
		:effect (and (AT ?g) (not (AT ?s)))))