// Catch'em - envio del tiempo ganador a la tabla de records.

#include "CatchemRecords.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


// ===================== CONFIGURACION =====================
// Pega aca la URL que termina en /exec que te dio Apps Script,
// y la misma CLAVE que pusiste en el script.

static const FString URL_RECORDS   = TEXT("https://script.google.com/macros/s/AKfycbwgo2hJU1gmHHBecCYLjTJtwgfOtxrb0JO8FqhwLOCMHFmPCLQXj5sdt-wP_DSUt6QI/exec");
static const FString CLAVE_RECORDS = TEXT("123456");

// =========================================================


// El nombre lo escribe una persona: si mete comillas o una barra
// invertida, el JSON se rompe. Esto las neutraliza.
static FString EscaparJson(const FString& Texto)
{
	FString Salida = Texto;
	Salida.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
	Salida.ReplaceInline(TEXT("\""), TEXT("\\\""));
	Salida.ReplaceInline(TEXT("\r"), TEXT(""));
	Salida.ReplaceInline(TEXT("\n"), TEXT(""));
	Salida.ReplaceInline(TEXT("\t"), TEXT(" "));
	return Salida;
}


void UCatchemRecords::EnviarTiempo(const FString& Jugador, float Segundos)
{
	if (Segundos <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Records] Tiempo invalido (%f), no se envia."), Segundos);
		return;
	}

	if (URL_RECORDS.Contains(TEXT("PEGA_ACA")))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Records] Falta configurar URL_RECORDS en CatchemRecords.cpp"));
		return;
	}

	const FString Cuerpo = FString::Printf(
		TEXT("{\"clave\":\"%s\",\"jugador\":\"%s\",\"segundos\":%.2f}"),
		*EscaparJson(CLAVE_RECORDS),
		*EscaparJson(Jugador),
		Segundos);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Peticion = FHttpModule::Get().CreateRequest();
	Peticion->SetURL(URL_RECORDS);
	Peticion->SetVerb(TEXT("POST"));

	// text/plain a proposito: Apps Script lee el cuerpo igual, y asi el
	// mismo endpoint sirve despues desde el navegador sin preflight.
	Peticion->SetHeader(TEXT("Content-Type"), TEXT("text/plain; charset=utf-8"));
	Peticion->SetTimeout(10.f);
	Peticion->SetContentAsString(Cuerpo);

	Peticion->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr Peticion, FHttpResponsePtr Respuesta, bool bHuboRespuesta)
		{
			if (!bHuboRespuesta || !Respuesta.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("[Records] Sin respuesta del servidor. El tiempo no se guardo."));
				return;
			}

			UE_LOG(LogTemp, Log, TEXT("[Records] HTTP %d - %s"),
				Respuesta->GetResponseCode(),
				*Respuesta->GetContentAsString());
		});

	Peticion->ProcessRequest();

	UE_LOG(LogTemp, Log, TEXT("[Records] Enviando: %s"), *Cuerpo);
}
