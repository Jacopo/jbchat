var url_fcgi = 'jbchat.fcgi';

var invio_abilitato = true;
var ultimo_messaggio_ricevuto = 0;

$(document).ready(function() {
	// Reset iniziale
	abilita_invio();

	// Predispone il gestore dell'invio dei messaggi
	$('#form_invio').submit(invia_messaggio);

	// Inizia l'ascolto dei messaggi
	richiedi_messaggi();
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
	if (xml != '<keepalive></keepalive>') {
		$(xml).find('msg').each(function() {
			// TODO: come invertire il form-encoding?
			//       Vanno anche cambiati + in spazi, etc
			//       Il browser rifiuta di aggiungere il testo come UTF-8
			// Forse vale la pena di passare a un altro encoding per il post
			var testo = unescape($(this).text());
			var autore = unescape($(this).attr('autore'));

			var numero = $(this).attr('numero');
			var RE_num = /^\d+$/;
			if (RE_num.test(numero) == false) {
				mostra_stato("Errore ricezione numero messaggio", 0);
				return;
			}
			numero = parseInt(numero);
			if (numero > ultimo_messaggio_ricevuto)
				ultimo_messaggio_ricevuto = numero;

			// Nota: text(stringa) si occupa anche
			//       dell'escaping
			var nuovo_nodo = $('<li></li>').text(testo);
			$('<div class="autore"></div>').text(autore).appendTo(nuovo_nodo);
			nuovo_nodo.appendTo("#msgs");
		});
	}
	richiedi_messaggi();
}

function errore_ricezione(XMLHttpRequest, textStatus, errorThrown)
{
	mostra_stato("Si è verificato un errore di ricezione (" + textStatus + "), ricaricare la pagina", 1);
}


// Invio messaggi ////////////////////////////////////////
function invia_messaggio()
{
	if (!invio_abilitato)
		return false;

	disabilita_invio();
	$.ajax({
		type: 'POST',
		url: url_fcgi,
		data: $('#form_invio').serialize(),
		success: function(risp) {
			if (risp == 'OK')
				abilita_invio();
			else errore_invio();
			},
		error: errore_invio,
		dataType: 'text',
		timeout: 1000
	});
	return false;
}

function disabilita_invio()
{
	invio_abilitato = false;
	$('#bottone_invio')
		.attr('disabled', true)
		.val('Invio in corso...');
}

function abilita_invio()
{
	$('#testo').val("");
	invio_abilitato = true;
	$('#bottone_invio')
		.attr('disabled', false)
		.val('Invia');
}

function errore_invio(XMLHttpRequest, textStatus, errorThrown)
{
	mostra_stato("Si è verificato un errore durante l'invio del messaggio: " + textStatus, 2);
	abilita_invio();
}


// Funzioni di interfaccia ///////////////////////////////
function mostra_stato(s, tipo)
{
	// tipo:
	//        0   normale
	//        1   errore permanente
	//        2   errore temporaneo

	// TODO: stili, fadeout
	$("#stato").text(s);
}

