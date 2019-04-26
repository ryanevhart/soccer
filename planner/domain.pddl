(define (domain SOCCER)
	(:requirements :strips :equality)
	
	(:predicates 
  		(at ?player ?pos)
  		(hasball ?player)
    	(connected ?s ?g)
    	(touchdown)
    	(close ?one ?two))

	(:action move
		:parameters (?p ?s ?g)
		:precondition (and (or (connected ?s ?g) (connected ?g ?s)) (at ?p ?s))
		:effect (and (at ?p ?g) (not (at ?p ?s))))

	(:action shoot
		:parameters (?p)
		:precondition (and (or (at ?p DI) (at ?p EI) (at ?p FI) (at ?p GI) (at ?p DJ) (at ?p EJ) (at ?p FJ) (at ?p GJ)) (hasball ?p))
		:effect (touchdown))

	(:action pass
		:parameters (?one ?two)
		:precondition (and (close ?one ?two) (hasball ?one))
		:effect (and (not (hasball ?one)) (hasball ?two))))