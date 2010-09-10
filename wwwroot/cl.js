var url_fcgi = 'jbchat.fcgi';

var ultimo_messaggio_ricevuto = 0;
var messaggi_in_coda = new Array();
var ritardo_per_tentativi_ricezione = 1;
var ritardo_per_tentativi_invio = 1;

$(document).ready(function() {
	// Reset iniziale
	$("#stato").hide();

	// Predispone il gestore dell'invio dei messaggi
	$('#form_invio').submit(accoda_messaggio);

	// Inizia l'ascolto dei messaggi
	// (usiamo un timer per indicare al browser che il caricamento
	//  della pagina è stato completato)
	setTimeout(richiedi_messaggi, 1000);
});


// Ricezione messaggi ////////////////////////////////////
function richiedi_messaggi()
{
	$.ajax({
		type: 'GET',
		url: url_fcgi,
		data: { 'ricevi_da' : ultimo_messaggio_ricevuto+1 },
		success: arrivo_messaggi,
		error: errore_ricezione,
		dataType: 'xml',
		timeout: 60000
	});
}

function arrivo_messaggi(xml)
{
	ritardo_per_tentativi_ricezione = 1;
	if (xml != '<keepalive></keepalive>') {
		$(xml).find('msg').each(function() {
			var testo = decodeURIComponent($(this).text());
			var autore = decodeURIComponent($(this).attr('autore'));

			var numero = $(this).attr('numero');
			var RE_num = /^\d+$/;
			if (RE_num.test(numero) == false) {
				mostra_stato("Errore ricezione numero messaggio", 0);
				return;
			}
			numero = parseInt(numero);
			if (numero > ultimo_messaggio_ricevuto)
				ultimo_messaggio_ricevuto = numero;
			else mostra_stato("Messaggi ricevuti fuori ordine (" + ultimo_messaggio_ricevuto + "prima di " + numero + "), si consiglia di ricaricare la pagina", 0);

			// Nota: text(stringa) si occupa anche
			//       dell'escaping
			var nuovo_nodo = $('<li></li>').text(testo);
			$('<div class="autore"></div>').text(autore).appendTo(nuovo_nodo);
			nuovo_nodo.appendTo("#msgs");
		});
		$('#msgs').animate({scrollTop: $('#msgs')[0].scrollHeight});
	}
	richiedi_messaggi();
}

function errore_ricezione(XMLHttpRequest, textStatus, errorThrown)
{
	// Riprovo la ricezione dei messaggi, aumentando il ritardo ad ogni tentativo

	if (ritardo_per_tentativi_ricezione >= 200) {
		mostra_stato("Impossibile ricevere messaggi (erano in coda per l'invio: " + messaggi_in_coda.length + "): " + textStatus + ". Ricaricare la pagina", 0);
		return;
	}

	mostra_stato("Si è verificato un errore di ricezione (riprovo tra " + ritardo_per_tentativi_ricezione + " secondi): " + textStatus, ritardo_per_tentativi_ricezione);
	setTimeout(richiedi_messaggi, ritardo_per_tentativi_ricezione * 1000);
	ritardo_per_tentativi_ricezione *= 2;
}


// Invio messaggi ////////////////////////////////////////
function accoda_messaggio()
{
	var testo_encoded = encodeURIComponent($("#testo").val());
	var autore_encoded = encodeURIComponent($("#autore").val());
	$("#testo").val("");

	var dati = "testo=" + testo_encoded + "&autore=" + autore_encoded;
	messaggi_in_coda.push(dati);
	invia_messaggio_testa();

	return false;
}

function invia_messaggio_testa()
{
	if (messaggi_in_coda.length == 0)
		return;

	$.ajax({
		type: 'POST',
		url: url_fcgi,
		data: messaggi_in_coda[0],
		success: function(risp) {
				if (risp == 'OK') {
					ritardo_per_tentativi_invio = 1;
					messaggi_in_coda.shift();
					invia_messaggio_testa();
				} else errore_invio();
			},
		error: errore_invio,
		dataType: 'text',
		timeout: 1000
	});
}

function errore_invio(XMLHttpRequest, textStatus, errorThrown)
{
	// Riprovo l'invio del messaggio, aumentando il ritardo ad ogni tentativo

	if (ritardo_per_tentativi_invio >= 200) {
		mostra_stato("Impossibile inviare i messaggi (erano in coda: " + messaggi_in_coda.length + ", non saranno fatti altri tentativi): " + textStatus, 0);
		return;
	}

	mostra_stato("Si è verificato un errore durante l'invio del messaggio (in coda: " + messaggi_in_coda.length + ", riprovo tra " + ritardo_per_tentativi_invio + " secondi): " + textStatus, ritardo_per_tentativi_invio);
	setTimeout(invia_messaggio_testa, ritardo_per_tentativi_invio * 1000);
	ritardo_per_tentativi_invio *= 2;
}


// Funzioni di interfaccia ///////////////////////////////
function mostra_stato(s, tempo)
{
	$("#stato").text(s);
	if (tempo != 0)
		$("#stato").fadeIn(100).delay(tempo*1000-700).fadeOut(600);
	else $("#stato").fadeIn(100);
}

