# Out-Of-Band-Signaling.

Si tratta di una tecnica di comunicazione in cui due entità si scambiano informazioni senza trasmettersele direttamente, ma utilizzando segnalazione collaterale.
E' un sisteama client-server, in cui i client possiedono un codice segreto e vogliono comunicarlo a un server centrale, senza però trasmetterlo.
Lo scopo è rendere difficile intercettare il secret a chi stia catturando i dati in transito.

In questo progetto abbiamo C client, S server, e 1 supervisor.
Viene lanciato per primo il supervisor, con un parametro S che indica quanti server vogliamo attivare; il supervisor provvederà dunque a creare i S server (processi distinti). Gli C client vengono invece lanciati indipendentemente, ciascuno in tempi diversi.

Tutto il codice segue lo standard POSIX.

## Requisiti
Per eseguire il codice bisogna installare:
<ul>
  <li>gcc</li>
  <li>make</li>
</ul>

## Comandi

Comandi che possono essere eseguiti:  <br> <br>
``` make ```  - per compilare tutti i file <br> <br>
``` make test ```  - per eseguire una routine di test <br> <br>
``` make clean ```  - per pulire la directory <br> <br>

## Configurazione

Prima di eseguire la routine di test eseguire i seguenti comandi:
<ul>
  <li>chmod 777 misura.sh</li>
  <li>sed -i -e 's/\r$//' misura.sh</li>
</ul>
