let ESP_IP = "";

let audioLiberado = false;

let timerSensores = null;

let falhasPing = 0;

const MAX_FALHAS = 3;


// ================================
// FETCH COM TIMEOUT
// ================================
async function fetchComTimeout(url, tempo = 1000)
{
    const controller =
        new AbortController();

    const timeout =
        setTimeout(
            () => controller.abort(),
            tempo
        );

    try
    {
        const resposta =
            await fetch(url,{
                signal: controller.signal
            });

        clearTimeout(timeout);

        return resposta;

   /*  }catch(erro){

        clearInterval(timerSensores);

        interfaceOffline();

        document
        .getElementById("statusConexao")
        .innerHTML = "ESP32 OFFLINE";

        document
        .getElementById("panelConnect")
        .style.display = "flex";

        throw erro;
    } */
	
	}catch(erro){

    throw erro;
}
	
	
	
}


// ================================
// CONECTAR ESP32
// ================================
async function conectarESP32()
{
    const ip =
        document.getElementById("ip").value;

    const status =
        document.getElementById("statusConexao");

    status.innerHTML = "Conectando...";

    try
    {
        const resposta =
            await fetch(`http://${ip}/ping`);

        const texto =
            await resposta.text();

        if (texto.trim().toUpperCase() === "ONLINE")
        {
			falhasPing = 0;
			
            ESP_IP = ip;
			
			document
            .getElementById("ipServidor")
            .innerHTML = `ESP32: ${ip}`;

            status.innerHTML =
                "ESP32 ONLINE";

            interfaceOnline();

            document
            .getElementById("panelConnect")
            .style.display = "none";

            clearInterval(timerSensores);

            timerSensores =
                setInterval(atualizarTudo,1000);

            atualizarTudo();

        }else{

                interfaceOffline();

    document
    .getElementById("statusConexao")
    .innerHTML = "ESP32 OFFLINE";

    document
    .getElementById("panelConnect")
    .style.display = "flex";
        }

    }catch(erro){

        status.innerHTML =
            "ESP32 OFFLINE";

        interfaceOffline();
    }
}


// ================================
// TEMPERATURA
// ================================
async function obterTemperatura()
{
    try
    {
        const resposta =
            await fetchComTimeout(
                `http://${ESP_IP}/temperatura`
            );

        const valor =
            await resposta.text();

        const valorTemp =
            parseFloat(valor);

        if(isNaN(valorTemp))
            return;

        document
        .getElementById("temperatura")
        .innerHTML =
            valorTemp.toFixed(1) + "&deg;C";

        const circulo =
            document.querySelector(".temp-progress");

        const percentual =
            valorTemp / 50;

        const offset =
            565 - (565 * percentual);

        circulo.style.strokeDashoffset =
            offset;

    }catch{}
}


// ================================
// PROXIMIDADE
// ================================
async function obterProximidade()
{
    try
    {
        const resposta =
            await fetchComTimeout(
                `http://${ESP_IP}/proximidade`
            );

        const valor =
            await resposta.text();

        document
        .getElementById("proximidade")
        .innerHTML =
            valor + " cm";

    }catch{}
}


// ================================
// POTENCIOMETRO
// ================================
async function obterPotenciometro()
{
    try
    {
        const resposta =
            await fetchComTimeout(
                `http://${ESP_IP}/potenciometro`
            );

        const valor =
            await resposta.text();

        document
        .getElementById("potenciometro")
        .innerHTML =
            valor;

        const barra =
            document.getElementById("pot-bar");

        const percentual =
            (parseInt(valor) / 4095) * 100;

        barra.style.width =
            percentual + "%";

    }catch{}
}


// ================================
// PRESENÇA
// ================================
async function obterPresenca()
{
    try
    {
        const resposta =
            await fetchComTimeout(
                `http://${ESP_IP}/presenca`
            );

        const valor =
            await resposta.text();

        document
        .getElementById("presenca")
        .innerHTML =
            valor;

        const presenca =
            document.getElementById("presenca");

        if(valor.includes("Detectada"))
        {
            presenca.classList.add(
                "presenca-ativa"
            );

        }else{

            presenca.classList.remove(
                "presenca-ativa"
            );
        }

    }catch{}
}


// ================================
// ATUALIZA TUDO
// ================================
function atualizarTudo()
{
    obterTemperatura();

    obterProximidade();

    obterPotenciometro();

    obterPresenca();
}


// ================================
// STATUS ONLINE/OFFLINE
// ================================
function atualizarStatus(conectado)
{
    const status =
        document.querySelector(".status");

    if(conectado)
    {
        status.innerHTML =
            "&#9679; CONECTADO";

        status.style.color =
            "#00ff88";

    }else{

        status.innerHTML =
            "&#9679; DESCONECTADO";

        status.style.color =
            "red";
    }
}


// ================================
// INTERFACE OFFLINE
// ================================
function interfaceOffline()
{
  atualizarStatus(false);

    clearInterval(timerSensores);

    if(audioLiberado)
    {
        const som =
            document.getElementById(
                "offlineSound"
            );

        som.currentTime = 0;

        som.volume = 0.5;

        som.play();
    }

    // TEXTO OFFLINE
    document
    .getElementById("statusConexao")
    .innerHTML =
    `
        <div style="
            font-size:40px;
            color:white;
            font-weight:bold;
            margin-bottom:15px;
        ">
            ESP32 OFFLINE
        </div>

        <div style="
            color:#cbd5e1;
            font-size:18px;
        ">
            Tentando reconectar...
        </div>
    `;

    // MOSTRA MODAL
    document
    .getElementById("panelConnect")
    .style.display = "flex";

    // REABRE LOGIN APÓS 4 SEGUNDOS
    setTimeout(()=>{

        document
        .getElementById("statusConexao")
        .innerHTML = "";

    },4000);
}


// ================================
// INTERFACE ONLINE
// ================================
function interfaceOnline()
{
    atualizarStatus(true);

    document
    .querySelectorAll("input,button")
    .forEach(elemento=>{

        elemento.disabled = false;

        elemento.style.opacity = "1";

    });

    document.body.style.opacity =
        "1";

    document.body.style.filter =
        "none";

/*     document
    .getElementById("offlineOverlay")
    .classList.remove("show"); */
}


// ================================
// LED
// ================================
async function toggleLED(elemento)
{
    const estado =
        elemento.checked ? "on" : "off";

    document
    .getElementById("clickSound")
    .play();

    await fetchComTimeout(
        `http://${ESP_IP}/led?estado=${estado}`
    );
}


// ================================
// RELÉS
// ================================
async function toggleRele(id, elemento)
{
    const estado =
        elemento.checked ? "on" : "off";

    document
    .getElementById("clickSound")
    .play();

    await fetchComTimeout(
        `http://${ESP_IP}/rele${id}?estado=${estado}`
    );
}


// ================================
// BUZZER
// ================================
async function acionarBuzzer()
{
    document
    .getElementById("clickSound")
    .play();

    await fetchComTimeout(
        `http://${ESP_IP}/buzzer`
    );
}


// ================================
// SOM HOVER
// ================================
const hoverSound =
    document.getElementById("hoverSound");

document
.querySelectorAll(".card,.controle-card")
.forEach(card=>{

    card.addEventListener(
        "mouseenter",
        ()=>{

            hoverSound.currentTime = 0;

            hoverSound.volume = 0.2;

            hoverSound.play();
        }
    );

});


// ================================
// LIBERA AUDIO
// ================================
document.addEventListener(
    "click",
    ()=>{

        audioLiberado = true;

    },
    { once:true }
);


// ================================
// BOTÃO CONECTAR
// ================================
document
.getElementById("btnConectar")
.addEventListener(
    "click",
    conectarESP32
);


console.log(
    "Dashboard iniciado com sucesso"
);

/* setInterval(async ()=>{

    if(ESP_IP == "")
        return;

    try
    {
        const resposta =
            await fetch(
                `http://${ESP_IP}/ping`
            );

        const texto =
            await resposta.text();
			
			console.log("PING:", texto);


        if(texto.trim().toUpperCase() != "ONLINE")
        {
            clearInterval(timerSensores);

            interfaceOffline();

            document
            .getElementById("statusConexao")
            .innerHTML = "ESP32 OFFLINE";

            document
            .getElementById("panelConnect")
            .style.display = "flex";
        }

    }catch{}
    
},2000); */

setInterval(async ()=>{

    if(ESP_IP == "")
        return;

    try
    {
        const resposta =
            await fetchComTimeout(
                `http://${ESP_IP}/ping`,
                2000
            );

        const texto =
            await resposta.text();

        if(texto.trim().toUpperCase() === "ONLINE")
        {
            falhasPing = 0;

            return;
        }

        falhasPing++;

    }catch{

        falhasPing++;
    }

    // Só derruba após várias falhas
    if(falhasPing >= MAX_FALHAS)
    {
        clearInterval(timerSensores);

        interfaceOffline();

        document
        .getElementById("statusConexao")
        .innerHTML = "ESP32 OFFLINE";

        document
        .getElementById("panelConnect")
        .style.display = "flex";
    }

},3000);