const galleryEventDates = [
  "2026-08-04",
  "2026-08-05",
  "2026-08-06",
  "2026-08-07",
];

let gallerySelectedDate =
  null;

let galleryRefreshTimer =
  null;

let galleryLastSignatureCount =
  -1;  

let gallerySignatureIndex =
  0;

let gallerySignatureTimer =
  null; 
  
let galleryPhotoIndex =
  0;

let galleryPhotoTimer =
  null;

let galleryLastPhotoCount =
  -1;

let serialMonitorFilter =
  "ALL";  

// ======================================================
// CARROSSEL DE FOTOS DO MURAL
// ======================================================

function changeGalleryPhoto(
  photos,
  eventDate,
  newIndex
) {
  const carousel =
    qs("#galleryPhotosCarousel");

  const track =
    carousel?.querySelector(
      ".gallery-photo-track"
    );

  if (!track) {
    changeGalleryPhoto(
  photos,
  eventDate,
  nextIndex
);

    return;
  }

  track.classList.add(
    "is-changing"
  );

  setTimeout(
    () => {
      galleryPhotoIndex =
        newIndex;

      updateGalleryPhotoCarousel(
        photos,
        eventDate
      );

      const newTrack =
        carousel.querySelector(
          ".gallery-photo-track"
        );

      if (!newTrack) {
        return;
      }

      newTrack.classList.add(
        "is-changing"
      );

      requestAnimationFrame(
        () => {
          requestAnimationFrame(
            () => {
              newTrack.classList.remove(
                "is-changing"
              );
            }
          );
        }
      );
    },
    280
  );
}

function stopGalleryPhotoCarousel() {
  if (galleryPhotoTimer) {
    clearInterval(
      galleryPhotoTimer
    );

    galleryPhotoTimer =
      null;
  }
}

function updateGalleryPhotoCarousel(
  photos,
  eventDate
) {
  const carousel =
    qs("#galleryPhotosCarousel");

  if (
    !carousel ||
    !Array.isArray(photos) ||
    photos.length === 0
  ) {
    return;
  }

  if (
    galleryPhotoIndex >=
    photos.length
  ) {
    galleryPhotoIndex =
      0;
  }

  const totalPhotos =
  photos.length;

 const currentIndex =
  galleryPhotoIndex;

 const previousIndex =
  (
    currentIndex -
    1 +
    totalPhotos
  ) %
  totalPhotos;

 const nextIndex =
  (
    currentIndex +
    1
  ) %
  totalPhotos;


 const currentFile =
  photos[currentIndex];

 const previousFile =
  photos[previousIndex];

 const nextFile =
  photos[nextIndex];


 const photoUrl =
  fileName =>
    `/api/gallery/photos/file?date=${encodeURIComponent(
      eventDate
    )}&name=${encodeURIComponent(
      fileName
    )}`;


 carousel.innerHTML = `

  <div class="gallery-photo-track">

    ${
      totalPhotos > 1
        ? `
          <button
            type="button"
            class="
              gallery-photo-preview
              gallery-photo-previous
            "
            data-gallery-action="previous"
            title="Foto anterior"
          >
            <img
              src="${photoUrl(
                previousFile
              )}"
              alt="Foto anterior"
            >
          </button>
        `
        : ""
    }


    <div class="gallery-photo-current">

      <img
        class="gallery-photo-main"
        src="${photoUrl(
          currentFile
        )}"
        alt="Momento do C++ Experience"
      >

      <div class="gallery-photo-position">
        ${
          currentIndex + 1
        } de ${totalPhotos}
      </div>

    </div>


    ${
      totalPhotos > 1
        ? `
          <button
            type="button"
            class="
              gallery-photo-preview
              gallery-photo-next
            "
            data-gallery-action="next"
            title="Próxima foto"
          >
            <img
              src="${photoUrl(
                nextFile
              )}"
              alt="Próxima foto"
            >
          </button>
        `
        : ""
    }

  </div>
 `;
 carousel
  .querySelector(
    '[data-gallery-action="previous"]'
  )
  ?.addEventListener(
    "click",
    () => {
      galleryPhotoIndex =
        previousIndex;

      renderGalleryPhotoCarousel(
        photos,
        eventDate
      );
    }
  );


 carousel
  .querySelector(
    '[data-gallery-action="next"]'
  )
  ?.addEventListener(
    "click",
    () => {
      changeGalleryPhoto(
      photos,
      eventDate,
      previousIndex
    );
    }
  );

}

function startGalleryPhotoCarousel(
  photos,
  eventDate
) {
  stopGalleryPhotoCarousel();

  if (
    !Array.isArray(photos) ||
    photos.length <= 1
  ) {
    return;
  }

  galleryPhotoTimer =
    setInterval(
      () => {
        if (
          state.currentView !==
          "gallery"
        ) {
          return;
        }

        const nextIndex =
  (
    galleryPhotoIndex + 1
  ) %
  photos.length;

changeGalleryPhoto(
  photos,
  eventDate,
  nextIndex
);
      },
      4000
    );
}

// ======================================================
// SELETORES
// ======================================================

const qs = (selector, parent = document) =>
  parent.querySelector(selector);

const qsa = (selector, parent = document) =>
  [...parent.querySelectorAll(selector)];

// ======================================================
// ESTADO DA APLICAÇÃO
// ======================================================

const state = {
  connected: false,
  started: Date.now(),
  chart: Array.from({ length: 40 }, () => 0),

  currentView: "dashboard",
  devices: [],
  
  shieldsMode: "devices",
  selectedShieldDeviceId: null
};

const toastStack = qs("#toastStack");

// ======================================================
// NOTIFICAÇÕES
// ======================================================

function toast(title, message) {
  if (!toastStack) {
    console.log(`${title}: ${message}`);
    return;
  }

  const element = document.createElement("div");

  element.className = "toast";

  element.innerHTML = `
    <strong>${title}</strong>
    <p>${message}</p>
  `;

  toastStack.appendChild(element);

  setTimeout(() => {
    element.style.opacity = "0";
    element.style.transform = "translateX(25px)";

    setTimeout(() => {
      element.remove();
    }, 300);
  }, 3200);
}

// ======================================================
// RELÓGIO
// ======================================================

function updateClock() {
  const now = new Date();

  const lastTime = qs("#lastTime");
  const lastDate = qs("#lastDate");
  const uptime = qs("#uptime");

  if (lastTime) {
    lastTime.textContent =
      now.toLocaleTimeString("pt-BR");
  }

  if (lastDate) {
    lastDate.textContent =
      now.toLocaleDateString("pt-BR");
  }

  const difference =
    Date.now() - state.started;

  const hours = String(
    Math.floor(difference / 36e5),
  ).padStart(2, "0");

  const minutes = String(
    Math.floor((difference % 36e5) / 6e4),
  ).padStart(2, "0");

  const seconds = String(
    Math.floor((difference % 6e4) / 1e3),
  ).padStart(2, "0");

  if (uptime) {
    uptime.textContent =
      `${hours}:${minutes}:${seconds}`;
  }
}

// ======================================================
// ATUALIZAÇÃO DOS CARDS
// ======================================================

function recalc() {
  const cards = qsa(".shield-card");

  const activeCards = cards.filter((card) => {
    const input = qs("input", card);

    return input?.checked === true;
  });

  const consumption = activeCards.reduce(
    (total, card) => {
      return (
        total +
        Number(card.dataset.consumption || 0)
      );
    },
    0,
  );

  const activeCount =
    qs("#activeCount");

  const currentConsumption =
    qs("#currentConsumption");

  const consumptionPercent =
    qs("#consumptionPercent");

  const consumptionBar =
    qs("#consumptionBar");

  const signalsOk =
    qs("#signalsOk");

  if (activeCount) {
    activeCount.textContent =
      activeCards.length;
  }

  if (currentConsumption) {
    currentConsumption.textContent =
      consumption;
  }

  const percentage = Math.min(
    100,
    Math.round((consumption / 500) * 100),
  );

  if (consumptionPercent) {
    consumptionPercent.textContent =
      percentage;
  }

  if (consumptionBar) {
    consumptionBar.style.setProperty(
      "--value",
      `${percentage}%`,
    );
  }

  if (signalsOk) {
    signalsOk.textContent =
      activeCards.length;
  }

  cards.forEach((card) => {
    const input =
      qs("input", card);

    if (!input) {
      return;
    }

    const checked =
      input.checked;

    card.classList.toggle(
      "active-card",
      checked,
    );

    card.classList.toggle(
      "inactive-card",
      !checked,
    );

    const stateElement =
      qs(".state", card);

    if (stateElement) {
      stateElement.classList.toggle(
        "active",
        checked,
      );

      stateElement.innerHTML = `
        <i></i>
        ${checked ? "Ativo" : "Inativo"}
      `;
    }

    const consumptionText =
      qsa(".shield-copy p", card).find(
        (paragraph) =>
          paragraph.textContent.includes(
            "Consumo",
          ),
      );

    if (consumptionText) {
      consumptionText.textContent =
        `Consumo: ${
          checked
            ? card.dataset.consumption || 0
            : 0
        } mA`;
    }
  });
}

// ======================================================
// SWITCHES
// ======================================================

qsa(".switch input").forEach((input) => {
  input.addEventListener(
    "change",
    (event) => {
      recalc();

      const card =
        event.target.closest(
          ".shield-card",
        );

      if (!card) {
        return;
      }

      const enabled =
        event.target.checked;

      toast(
        enabled
          ? "Módulo ativado"
          : "Módulo desativado",
        `${card.dataset.name} foi ${
          enabled
            ? "habilitado"
            : "desabilitado"
        }.`,
      );
    },
  );
});

// ======================================================
// BOTÕES DE ATIVAÇÃO
// ======================================================

qsa(".activate-btn").forEach((button) => {
  button.addEventListener(
    "click",
    () => {
      const card =
        button.closest(
          ".shield-card",
        );

      if (!card) {
        return;
      }

      const input =
        qs("input", card);

      if (!input) {
        return;
      }

      input.checked = true;

      input.dispatchEvent(
        new Event("change"),
      );
    },
  );
});

// ======================================================
// CONFIGURAÇÃO DOS MÓDULOS
// ======================================================

qsa(".configure-btn").forEach(
  (button) => {
    button.addEventListener(
      "click",
      () => {
        const card =
          button.closest(
            ".shield-card",
          );

        if (!card) {
          return;
        }

        const name =
          card.dataset.name ||
          "Módulo";

        const modalTitle =
          qs("#modalTitle");

        const moduleId =
          qs("#moduleId");

        const modal =
          qs("#configModal");

        if (modalTitle) {
          modalTitle.textContent =
            name;
        }

        if (moduleId) {
          moduleId.value =
            `TSX-${name
              .toUpperCase()
              .replaceAll(" ", "-")}`;
        }

        if (modal?.showModal) {
          modal.showModal();
        }
      },
    );
  },
);

// ======================================================
// SALVAR CONFIGURAÇÃO
// ======================================================

qs("#saveConfigBtn")
  ?.addEventListener(
    "click",
    () => {
      toast(
        "Configuração salva",
        "As alterações foram aplicadas ao módulo.",
      );
    },
  );

// ======================================================
// TESTE VISUAL DOS MÓDULOS
// ======================================================

qsa(".test-btn").forEach((button) => {
  button.addEventListener(
    "click",
    () => {
      const card =
        button.closest(
          ".shield-card",
        );

      if (!card) {
        return;
      }

      button.disabled = true;

      const originalContent =
        button.innerHTML;

      button.innerHTML =
        "◌ Testando...";

      card.style.boxShadow =
        "0 0 35px rgba(0,190,255,.35)";

      setTimeout(() => {
        button.disabled = false;

        button.innerHTML =
          originalContent;

        card.style.boxShadow = "";

        toast(
          "Teste visual concluído",
          `${card.dataset.name} respondeu à animação da interface.`,
        );
      }, 1300);
    },
  );
});

// ======================================================
// DETECÇÃO DO DISPOSITIVO
// ======================================================

qs("#detectBtn")
  ?.addEventListener(
    "click",
    async () => {
      const button =
        qs("#detectBtn");

      const grid =
        qs("#shieldGrid");

      if (!button) {
        return;
      }

      button.disabled = true;

      button.innerHTML =
        "<span>◌</span> Procurando Arduino...";

      grid?.classList.add(
        "detecting",
      );

      try {
        const response =
          await fetch(
            "/api/device/scan",
            {
              method: "POST",
              cache: "no-store",
            },
          );

        if (!response.ok) {
          throw new Error(
            `HTTP ${response.status}`,
          );
        }

        toast(
          "Varredura iniciada",
          "O C++Builder está verificando a porta serial.",
        );
      } catch (error) {
        toast(
          "Rota ainda indisponível",
          "O endpoint /api/device/scan ainda precisa ser criado no C++Builder.",
        );

        console.error(
          "Erro ao iniciar a varredura:",
          error,
        );
      } finally {
        button.disabled = false;

        button.innerHTML =
          "<span>◌</span> Detectar dispositivo";

        grid?.classList.remove(
          "detecting",
        );
      }
    },
  );

// ======================================================
// BOTÃO DE CONEXÃO
// ======================================================

qs("#connectBtn")
  ?.addEventListener(
    "click",
    () => {
      toast(
        "Conexão automática",
        "A conexão será controlada pelo C++Builder através da porta serial.",
      );
    },
  );

// ======================================================
// DETALHES DO DISPOSITIVO
// ======================================================

qs("#deviceDetailsBtn")
  ?.addEventListener(
    "click",
    () => {
      toast(
        "Aguardando dispositivo",
        "Os detalhes serão carregados após a identificação da placa.",
      );
    },
  );

// ======================================================
// NAVEGAÇÃO
// ======================================================

qsa(".nav-item").forEach((item) => {
  item.addEventListener(
    "click",
    () => {
      qsa(".nav-item").forEach(
        (navigationItem) => {
          navigationItem.classList.remove(
            "active",
          );
        },
      );

      item.classList.add("active");

      const title =
        item.textContent.trim();

        if (
  title.includes("Dashboard")
) {
  showDashboardView();
  return;
}

if (
  title.includes("Shields")
) {
  showShieldsView();
  return;
}

if (
  title.includes("Dispositivos")
) {
  showDevicesView();
  return;
}

if (
  title.includes("Monitor Serial")
) {
  showSerialMonitorView();
  return;
}

if (
  title.includes("Mural")
) {
  showGalleryView();
  return;
}

   if (
  title.includes(
    "Sobre Mim"
  )
) {
  showAboutView();
  return;
}


      if (
        !title.includes("Shields")
      ) {
        toast(
          title,
          "Módulo da interface selecionado.",
        );
      }
    },
  );
});

// ======================================================
// GRÁFICO
// ======================================================

function drawChart() {
  const canvas =
    qs("#signalChart");

  if (!canvas) {
    return;
  }

  const context =
    canvas.getContext("2d");

  if (!context) {
    return;
  }

  const pixelRatio =
    window.devicePixelRatio || 1;

  const rectangle =
    canvas.getBoundingClientRect();

  if (
    rectangle.width === 0 ||
    rectangle.height === 0
  ) {
    return;
  }

  canvas.width =
    rectangle.width * pixelRatio;

  canvas.height =
    rectangle.height * pixelRatio;

  context.setTransform(
    pixelRatio,
    0,
    0,
    pixelRatio,
    0,
    0,
  );

  const width =
    rectangle.width;

  const height =
    rectangle.height;

  context.clearRect(
    0,
    0,
    width,
    height,
  );

  context.strokeStyle =
    "rgba(0,160,255,.12)";

  context.lineWidth = 1;

  for (
    let positionY = 15;
    positionY < height;
    positionY += 18
  ) {
    context.beginPath();

    context.moveTo(
      0,
      positionY,
    );

    context.lineTo(
      width,
      positionY,
    );

    context.stroke();
  }

  state.chart.shift();

  state.chart.push(
    state.connected
      ? 35 +
          Math.sin(
            Date.now() / 600,
          ) *
            10 +
          Math.random() * 28
      : 0,
  );

  context.beginPath();

  state.chart.forEach(
    (value, index) => {
      const positionX =
        (index /
          (state.chart.length - 1)) *
        width;

      const positionY =
        height -
        (value / 100) * height;

      if (index === 0) {
        context.moveTo(
          positionX,
          positionY,
        );
      } else {
        context.lineTo(
          positionX,
          positionY,
        );
      }
    },
  );

  const gradient =
    context.createLinearGradient(
      0,
      0,
      width,
      0,
    );

  gradient.addColorStop(
    0,
    "#006cff",
  );

  gradient.addColorStop(
    1,
    "#00d8ff",
  );

  context.strokeStyle =
    gradient;

  context.lineWidth = 2;

  context.shadowColor =
    "#00aaff";

  context.shadowBlur = 9;

  context.stroke();

  context.shadowBlur = 0;

  const lastValue =
    state.chart.at(-1) || 0;

  const pointX =
    width - 2;

  const pointY =
    height -
    (lastValue / 100) * height;

  context.fillStyle =
    "#22c9ff";

  context.beginPath();

  context.arc(
    pointX,
    pointY,
    4,
    0,
    Math.PI * 2,
  );

  context.fill();
}

// ======================================================
// ESTADO VISUAL: AGUARDANDO PLACA
// ======================================================

function showWaitingDevice() {
  state.connected = false;

  const capabilitiesPanel =
  qs("#deviceCapabilitiesPanel");

const defaultGrid =
  qs("#defaultShieldGrid");

  const deviceName =
  qs("#activeDeviceName");

const deviceId =
  qs("#activeDeviceId");

const deviceStatus =
  qs("#activeDeviceStatus");

const deviceStatusDot =
  qs("#activeDeviceStatusDot");

const deviceFirmware =
  qs("#activeDeviceFirmware");

const deviceProtocol =
  qs("#activeDeviceProtocol");

const deviceImage =
  qs("#activeDeviceImage");


if (deviceName) {
  deviceName.textContent =
    "Aguardando conexão";
}

if (deviceId) {
  deviceId.textContent =
    "Nenhuma placa detectada";
}

if (deviceStatus) {
  deviceStatus.textContent =
    "Aguardando";
}

if (deviceFirmware) {
  deviceFirmware.textContent =
    "-";
}

if (deviceProtocol) {
  deviceProtocol.textContent =
    "-";
}

if (deviceStatusDot) {
  deviceStatusDot.style.background =
    "#7b8794";

  deviceStatusDot.style.boxShadow =
    "none";
}

if (deviceImage) {
  deviceImage.style.opacity =
    "0.20";
}

if (capabilitiesPanel) {
  capabilitiesPanel.innerHTML = "";
  capabilitiesPanel.style.display = "none";
  capabilitiesPanel.dataset.device = "";
}

  if (defaultGrid) {
  defaultGrid.style.display = "none";
  }

  const connectionLabel =
    qs("#connectionLabel");

  const statusDot =
    qs(
      ".connection-status .status-dot",
    );

  const portSelect =
    qs("#portSelect");

   const shieldsTitle =
    qs("#connectedShieldsTitle");

   if (shieldsTitle) {
  shieldsTitle.textContent =
    "SHIELDS CONECTADOS";
   } 

  if (connectionLabel) {
    connectionLabel.textContent =
      "Aguardando placa";
  }

  if (statusDot) {
    statusDot.style.background =
      "#ffb020";

    statusDot.style.boxShadow =
      "0 0 12px rgba(255,176,32,.65)";
  }

  if (portSelect) {
    portSelect.innerHTML =
      '<option value="">Nenhuma porta conectada</option>';
  }


}






// ======================================================
// CONSULTA AO C++BUILDER
// ======================================================

async function loadDeviceStatus() {
  try {
    const response =
      await fetch(
        "/api/device/status",
        {
          method: "GET",
          cache: "no-store",
          headers: {
            Accept:
              "application/json",
          },
        },
      );

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`,
      );
    }

    const data =
      await response.json();

      if (!data?.connected) {

  // updateActiveDevicesCard(
  //   null
  // );

  //showWaitingDevice();

  return;
}

// A partir daqui sabemos que existe
// uma placa realmente conectada.
// updateActiveDevicesCard(
//   data
// );

state.connected = true;

      console.log(
  "[RIW] STATUS COMPLETO:",
  data
);

console.log(
  "[RIW] DEVICE:",
  data?.device
);

console.log(
  "[RIW] NAME:",
  data?.device?.name
);

console.log(
  "[RIW] ID:",
  data?.device?.id
);

console.log(
  "[RIW] FIRMWARE:",
  data?.device?.firmware
);

console.log(
  "[RIW] CONNECTED:",
  data?.connected
);

    if (!data?.connected) {
  // A API /api/device/status é legado (single-device).
  // O estado real dos dispositivos agora vem de /api/devices.
  // Portanto, ela não deve mais limpar a interface.
     return;
    }

    state.connected = true;

    const connectionLabel =
      qs("#connectionLabel");

    const statusDot =
      qs(
        ".connection-status .status-dot",
      );

    const portSelect =
      qs("#portSelect");

    if (connectionLabel) {
      connectionLabel.textContent =
        "Conectado";
    }

    if (statusDot) {
      statusDot.style.background =
        "#29e882";

      statusDot.style.boxShadow =
        "0 0 12px rgba(41,232,130,.75)";
    }

    if (portSelect) {
      const port =
        data.serial?.port ||
        "Porta desconhecida";

      portSelect.innerHTML =
        `<option value="${port}">${port}</option>`;
    }

     if (
      state.currentView === "shields" &&
      state.selectedDeviceId
    ) {
  // Não permitimos mais que a API antiga
  // escolha automaticamente o UNO.
}
  } catch (error) {

  console.warn(
    "A rota /api/device/status ainda não está disponível.",
    error,
  );
}
}

// ======================================================
// INICIALIZAÇÃO
// ======================================================

function initializeApplication() {
  
  qs("#galleryBackBtn")
  ?.addEventListener(
    "click",
    leaveGalleryView
  );
  
  
  showWaitingDevice();

  updateClock();
  recalc();
  drawChart();

  loadDeviceStatus();
  loadConnectedDevices();

  bindGalleryDays();

  showDashboardView();

  function renderCapabilities(capabilities) {
  const grid = qs("#dynamicCapabilityGrid");

  if (!grid) {
    console.warn("[RIW] dynamicCapabilityGrid não encontrado.");
    return;
  }

  grid.innerHTML = "";

  if (!Array.isArray(capabilities) || capabilities.length === 0) {
    grid.innerHTML = "<p>Nenhuma capacidade encontrada.</p>";
    return;
  }

  capabilities.forEach((capability) => {
    const card = document.createElement("div");

    card.className = "shield-card";
    card.dataset.name = capability.name;
    card.dataset.capabilityId = capability.id;
    card.dataset.type = capability.type;
    card.dataset.pin = capability.pin;

    card.innerHTML = `
      <div class="shield-copy">
        <h3>${capability.name}</h3>
        <p>ID: ${capability.id}</p>
        <p>Tipo: ${capability.type}</p>
        <p>Pino: ${capability.pin}</p>
      </div>

      <div class="state ${capability.state ? "active" : ""}">
        <i></i>
        ${capability.state ? "Ativo" : "Inativo"}
      </div>
    `;

    grid.appendChild(card);
  });
}

  async function loadDeviceIdentity() {
  try {
    const response = await fetch("/api/device/identity", {
      method: "GET",
      cache: "no-store",
    });

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const identity = await response.json();

    console.log("[RIW] Identidade recebida:", identity);
    console.log("[RIW] Dispositivo:", identity.device);
    console.log("[RIW] Capabilities:", identity.capabilities);

    renderCapabilities(identity.capabilities);


    if (Array.isArray(identity.capabilities)) {
      identity.capabilities.forEach((capability) => {
        console.log(
          `[RIW] ${capability.id} | ${capability.name} | ` +
          `tipo=${capability.type} | pino=${capability.pin} | ` +
          `estado=${capability.state}`,
        );
      });
    }
  } catch (error) {
    console.error("[RIW] Falha ao carregar identidade:", error);
  }
}

  setInterval(
    updateClock,
    1000,
  );

  setInterval(
    drawChart,
    1200,
  );

  setInterval(
    loadDeviceStatus,
    1500,
  );

  setInterval(
  loadConnectedDevices,
  1500,
);

  setInterval(
  loadSerialMonitor,
  1000
);

  setInterval(
  loadEsp32Temperature,
  1000
);

setInterval(
  loadEsp32Presence,
  200
);

  window.addEventListener(
    "resize",
    drawChart,
  );
}

document.addEventListener(
  "DOMContentLoaded",
  initializeApplication,
);

function updateActiveDevice(data) {
  const device =
    data?.device || {};

  const deviceId =
    device.id || "";

  const deviceName =
    device.name ||
    "Dispositivo desconhecido";

  const firmware =
    device.firmware ||
    "-";

  const protocol =
    device.protocol ||
    "TSX-HW/1.0";

  const port =
    data?.serial?.port ||
    "-";

  const deviceData = {
    name:
      deviceName,

    id:
      deviceId,

    firmware,

    protocol,

    port,
  };

  switch (deviceId) {
    case "TSX-UNO-001":
      showUnoDevice(
        deviceData,
      );
      break;

    case "TSX-MEGA-001":
      showMegaDevice(
        deviceData,
      );
      break;

    case "TSX-ESP32-001":
      showEsp32Device(
        deviceData,
      );
      break;

    default:
      showGenericDevice(
        deviceData,
      );
      break;
  }
}

function updateDeviceCard({
  image,
  name,
  id,
  firmware,
  protocol,
}) {
  const imageElement =
    qs("#activeDeviceImage");

  const nameElement =
    qs("#activeDeviceName");

  const idElement =
    qs("#activeDeviceId");

  const firmwareElement =
    qs("#activeDeviceFirmware");

  const protocolElement =
    qs("#activeDeviceProtocol");

  const statusElement =
    qs("#activeDeviceStatus");

  const statusDot =
    qs("#activeDeviceStatusDot");

  if (imageElement) {
     imageElement.src = image;
     imageElement.alt = name;
     imageElement.style.opacity = "1";
  }

  if (nameElement) {
    nameElement.textContent =
      name || "Dispositivo";
  }

  if (idElement) {
    idElement.textContent =
      id || "-";
  }

  if (firmwareElement) {
    firmwareElement.textContent =
      firmware || "-";
  }

  if (protocolElement) {
    protocolElement.textContent =
      protocol || device.protocol;
  }

  if (statusElement) {
    statusElement.textContent =
      "Online";
  }

  if (statusDot) {
    statusDot.style.background =
      "#29e882";

    statusDot.style.boxShadow =
      "0 0 8px rgba(41,232,130,.75)";
  }
}

function showUnoDevice(device) {
  updateDeviceCard({
    image:
      "assets/arduino-uno.png",

    name:
      device.name ||
      "Arduino Uno",

    id:
      device.id,

    firmware:
      device.firmware,

    protocol:
      device.protocol,
  });

  const title =
  qs("#connectedShieldsTitle");

  if (title) {
    title.textContent =
    "SHIELDS CONECTADOS : TSX - Arduino UNO";
  }
  renderUnoCapabilities();
}

function showMegaDevice(device) {
  updateDeviceCard({
    image:
      "assets/arduino-mega.png",

    name:
      device.name ||
      "Arduino Mega 2560",

    id:
      device.id,

    firmware:
      device.firmware,

    protocol:
      device.protocol,
  });
  const title =
    qs("#connectedShieldsTitle");

  if (title) {
    title.textContent =
      "SHIELDS CONECTADOS : TSX - Arduino Mega 2560";
  }

  renderMegaCapabilities();
}

function showEsp32Device(device) {
  updateDeviceCard({
    image:
      "assets/esp32s3.png",

    name:
      device.name ||
      "ESP32-S3",

    id:
      device.id,

    firmware:
      device.firmware,

    protocol:
      device.protocol,
  });
}

function renderEsp32Capabilities() {
  const defaultGrid =
    qs("#defaultShieldGrid");

  const panel =
    qs("#deviceCapabilitiesPanel");

  const title =
    qs("#connectedShieldsTitle");

  if (!panel) {
    return;
  }

  panel.dataset.device =
    "TSX-ESP32-001";

  if (defaultGrid) {
    defaultGrid.style.display =
      "none";
  }

  if (title) {
    title.style.display =
      "";

    title.textContent =
      "RECURSOS CONECTADOS : TSX - ESP32-S3";
  }

  panel.style.display =
    "grid";

  panel.innerHTML = `

    <div class="uno-capability-card">

      <div class="uno-device-image">
        <img
          src="assets/relay-shield.png"
          alt="Relé 1">
      </div>

      <div class="uno-device-content">

        <h3>Relé 1</h3>

        <span
          class="capability-state off"
          id="esp32-state-relay1">
          Desligado
        </span>

        <button
          type="button"
          class="btn btn-primary esp32-relay-btn"
          data-relay="relay1"
          data-state="false">
          Ligar
        </button>

      </div>

    </div>


    <div class="uno-capability-card">

      <div class="uno-device-image">
        <img
          src="assets/relay-shield.png"
          alt="Relé 2">
      </div>

      <div class="uno-device-content">

        <h3>Relé 2</h3>

        <span
          class="capability-state off"
          id="esp32-state-relay2">
          Desligado
        </span>

        <button
          type="button"
          class="btn btn-primary esp32-relay-btn"
          data-relay="relay2"
          data-state="false">
          Ligar
        </button>

      </div>

    </div>

    <div class="uno-capability-card esp32-sensor-card">

  <div class="esp32-sensor-animation">

    <div class="temperature-animation">

      <div class="temperature-glow"></div>

      <div class="thermometer">

        <div class="thermometer-tube">
          <div class="thermometer-level"></div>
        </div>

        <div class="thermometer-bulb"></div>

      </div>

      <div class="temperature-particles">
        <span></span>
        <span></span>
        <span></span>
      </div>

    </div>

  </div>

  <div class="uno-device-content">

    <h3>Temperatura</h3>

    <strong
      class="esp32-temperature-value"
      id="esp32-temperature">
      -- °C
    </strong>

    <span class="esp32-sensor-description">
      Monitoramento ambiente
    </span>

  </div>

</div>

   <div class="uno-capability-card esp32-sensor-card">

  <div class="esp32-sensor-animation">

    <div
      class="presence-radar"
      id="esp32PresenceRadar">

      <div class="radar-circle radar-circle-1"></div>
      <div class="radar-circle radar-circle-2"></div>
      <div class="radar-circle radar-circle-3"></div>

      <div class="radar-sweep"></div>

      <div class="radar-center"></div>

      <div class="radar-target"></div>

    </div>

  </div>

   <div class="esp32-sensor-content">

    <h3>Presença</h3>

    <strong
      class="esp32-presence-value"
      id="esp32-presence">
      Não detectada
    </strong>

    <span class="esp32-sensor-description">
      Detecção em tempo real
    </span>

  </div>

</div>
      
  `;

  bindEsp32RelayControls();

  loadEsp32Temperature();

  loadEsp32Presence();
}

function bindEsp32RelayControls() {
  const buttons =
    document.querySelectorAll(
      ".esp32-relay-btn"
    );

  buttons.forEach(
    button => {
      button.onclick =
        async () => {
          const relay =
            button.dataset.relay;

          if (
            relay !== "relay1" &&
            relay !== "relay2"
          ) {
            return;
          }

          const currentState =
            button.dataset.state ===
            "true";

          const newState =
            !currentState;

          let command = "";

          if (relay === "relay1") {
            command =
              newState
                ? "RELAY1_ON"
                : "RELAY1_OFF";
          }
          else {
            command =
              newState
                ? "RELAY2_ON"
                : "RELAY2_OFF";
          }

          button.disabled =
            true;

          try {
            await sendDeviceCommand(
              "TSX-ESP32-001",
              command
            );

            button.dataset.state =
              String(newState);

            button.textContent =
              newState
                ? "Desligar"
                : "Ligar";

            const stateLabel =
              qs(
                `#esp32-state-${relay}`
              );

            if (stateLabel) {
              stateLabel.textContent =
                newState
                  ? "Ligado"
                  : "Desligado";

              stateLabel.classList.toggle(
                "on",
                newState
              );

              stateLabel.classList.toggle(
                "off",
                !newState
              );
            }

            console.log(
              "[RIW] Comando ESP32 enviado:",
              command
            );
          }
          catch (error) {
            console.error(
              "[RIW] Erro ESP32:",
              error
            );

            toast(
              "ESP32",
              error.message ||
              "Falha ao enviar comando."
            );
          }
          finally {
            button.disabled =
              false;
          }
        };
    }
  );
}


function showGenericDevice(device) {
  updateDeviceCard({
    image:
      "assets/device-generic.png",

    name:
      device.name ||
      "Dispositivo TSX",

    id:
      device.id,

    firmware:
      device.firmware,

    protocol:
      device.protocol,
  });
}

function renderUnoCapabilities() {
  const defaultGrid =
    qs("#defaultShieldGrid");

  const panel =
    qs("#deviceCapabilitiesPanel");

  if (!panel) {
    return;
  }

  if (defaultGrid) {
    defaultGrid.style.display =
      "none";
  }

  panel.style.display =
    "grid";

  panel.innerHTML = `
    <div class="uno-capability-card">
      <div class="uno-device-image">
        <img
          src="assets/led-shield.png"
          alt="LED Vermelho 1">
      </div>

      <div class="uno-device-content">
        <h3>LED Vermelho 1</h3>

        <span
          class="capability-state off"
          id="state-led1">
          Desligado
        </span>

        <button
          type="button"
          class="btn btn-primary uno-toggle-btn"
          data-capability="led1"
          data-state="false">
          Ligar
        </button>
      </div>
    </div>


    <div class="uno-capability-card">
      <div class="uno-device-image">
        <img
          src="assets/led-shield.png"
          alt="LED Vermelho 2">
      </div>

      <div class="uno-device-content">
        <h3>LED Amarelo 2</h3>

        <span
          class="capability-state off"
          id="state-led2">
          Desligado
        </span>

        <button
          type="button"
          class="btn btn-primary uno-toggle-btn"
          data-capability="led2"
          data-state="false">
          Ligar
        </button>
      </div>
     </div>


    <div class="uno-capability-card">
      <div class="uno-device-image">
        <img
          src="assets/led-shield.png"
          alt="LED Vermelho 3">
      </div>

      <div class="uno-device-content">
        <h3>LED Verde 3</h3>

        <span
          class="capability-state off"
          id="state-led3">
          Desligado
        </span>

        <button
          type="button"
          class="btn btn-primary uno-toggle-btn"
          data-capability="led3"
          data-state="false">
          Ligar
        </button>
      </div>
    </div>


    <div class="uno-capability-card">
      <div class="uno-device-image">
        <img
          src="assets/relay-shield.png"
          alt="Relé Principal">
      </div>

      <div class="uno-device-content">
        <h3>Relé Principal</h3>

        <span
          class="capability-state off"
          id="state-relay1">
          Desligado
        </span>

        <button
          type="button"
          class="btn btn-primary uno-toggle-btn"
          data-capability="relay1"
          data-state="false">
          Ligar
        </button>
      </div>
    </div>


    <div class="uno-capability-card">
      <div class="uno-device-image">
        <img
          src="assets/buzzer-shield.png"
          alt="Buzzer">
      </div>

      <div class="uno-device-content">
        <h3>Buzzer</h3>

        <span
          class="capability-state off"
          id="state-buzzer1">
          Inativo
        </span>

        <button
          type="button"
          class="btn btn-primary uno-toggle-btn"
          data-capability="buzzer1"
          data-state="false">
          Ativar
        </button>
      </div>
    </div>
  `;

  bindUnoControls();
}

function processEsp32StatusFromMonitorEntry(
  entry
) {
  if (!entry) {
    return;
  }

  /*
   * O texto da linha pode estar em campos diferentes,
   * dependendo da estrutura usada pelo C++.
   */
  const line =
    entry.message ??
    entry.data ??
    entry.content ??
    entry.line ??
    entry.text ??
    "";

  if (
    typeof line !== "string" ||
    !line.includes(
      '"type":"status"'
    ) ||
    !line.includes(
      '"device_id":"TSX-ESP32-001"'
    )
  ) {
    return;
  }

  try {
    /*
     * Remove o prefixo:
     *
     * [COM5] [TSX-ESP32-001]
     *
     * e mantém somente o JSON.
     */
    const jsonStart =
      line.indexOf("{");

    if (jsonStart < 0) {
      return;
    }

    const jsonText =
      line.substring(
        jsonStart
      );

    const status =
      JSON.parse(
        jsonText
      );

    updateEsp32Status(
      status
    );
  }
  catch (error) {
    console.warn(
      "[RIW] Falha ao interpretar status do ESP32:",
      error,
      line
    );
  }
}

function updateEsp32Status(status) {
  if (
    !status ||
    status.type !== "status" ||
    status.device_id !== "TSX-ESP32-001" ||
    !Array.isArray(status.capabilities)
  ) {
    return;
  }

  const temperature =
    status.capabilities.find(
      item =>
        item.id === "temperature"
    );

  const presence =
    status.capabilities.find(
      item =>
        item.id === "presence"
    );

  const relay1 =
    status.capabilities.find(
      item =>
        item.id === "relay1"
    );

  const relay2 =
    status.capabilities.find(
      item =>
        item.id === "relay2"
    );

  // Temperatura
  const temperatureElement =
    document.querySelector(
      "#esp32-temperature"
    );

  if (
    temperatureElement &&
    temperature &&
    Number.isFinite(
      Number(temperature.value)
    )
  ) {
    temperatureElement.textContent =
      `${Number(
        temperature.value
      ).toFixed(1)} °C`;
  }

  // Presença
  const presenceElement =
    document.querySelector(
      "#esp32-presence"
    );

  const presenceDetected =
    presence?.state === true;

  if (presenceElement) {
    presenceElement.textContent =
      presenceDetected
        ? "Detectada"
        : "Não detectada";

    presenceElement.classList.toggle(
      "detected",
      presenceDetected
    );

    presenceElement.classList.toggle(
      "not-detected",
      !presenceDetected
    );
  }

  const radar =
    document.querySelector(
      "#esp32PresenceRadar"
    );

  if (radar) {
    radar.classList.toggle(
      "detected",
      presenceDetected
    );
  }

  // Sincroniza os botões com o status real
  updateEsp32RelayButton(
    "relay1",
    relay1?.state === true
  );

  updateEsp32RelayButton(
    "relay2",
    relay2?.state === true
  );
}

function updateEsp32RelayButton(
  relayId,
  isOn
) {
  const button =
    document.querySelector(
      `.esp32-relay-btn[data-relay="${relayId}"]`
    );

  const stateLabel =
    document.querySelector(
      `#esp32-state-${relayId}`
    );

  if (button) {
    button.dataset.state =
      String(isOn);

    button.textContent =
      isOn
        ? "Desligar"
        : "Ligar";
  }

  if (stateLabel) {
    stateLabel.textContent =
      isOn
        ? "Ligado"
        : "Desligado";

    stateLabel.classList.toggle(
      "on",
      isOn
    );

    stateLabel.classList.toggle(
      "off",
      !isOn
    );
  }
}

async function sendDeviceCommand(
  deviceId,
  commandObject
) {
  /*
   * Gera exatamente o texto que funcionou
   * no Monitor Serial do Arduino IDE:
   *
   * {"command":"set","target":"led1","value":true}
   */
  const serialCommand =
  typeof commandObject ===
    "string"
      ? commandObject
      : JSON.stringify(
          commandObject
        );

  console.log(
    "[RIW] Enviando para serial:",
    serialCommand
  );

  const response =
    await fetch(
      "/api/serial/send",
      {
        method: "POST",

        cache: "no-store",

        headers: {
          "Content-Type":
            "application/json",

          Accept:
            "application/json"
        },

        body:
          JSON.stringify({
            device_id:
              deviceId,

            command:
              serialCommand
          })
      }
    );

  const responseText =
    await response.text();

  let result = {};

  try {
    result =
      responseText
        ? JSON.parse(
            responseText
          )
        : {};
  }
  catch {
    throw new Error(
      `Resposta inválida: ${responseText}`
    );
  }

  if (!response.ok) {
    throw new Error(
      result?.error ||
      `HTTP ${response.status}`
    );
  }

  if (
    result.success !== true
  ) {
    throw new Error(
      result?.error ||
      "O servidor não confirmou o envio."
    );
  }

  return result;
}

function bindUnoControls() {
  const buttons =
    document.querySelectorAll(
      ".uno-toggle-btn"
    );

  buttons.forEach(
    button => {

      /*
       * Usamos onclick em vez de
       * addEventListener para garantir
       * apenas um envio por clique.
       */
      button.onclick =
        async () => {
          const capability =
            button.dataset.capability;

          if (!capability) {
            return;
          }

          const currentState =
            button.dataset.state ===
            "true";

          const newState =
            !currentState;

          button.disabled =
            true;

          try {
            await sendDeviceCommand(
              "TSX-UNO-001",
              {
                command:
                  "set",

                target:
                  capability,

                value:
                  newState
              }
            );

            /*
             * Só altera a interface depois
             * de o backend confirmar o envio.
             */
            button.dataset.state =
              String(
                newState
              );

            const stateLabel =
              qs(
                `#state-${capability}`
              );

            const isBuzzer =
              capability ===
              "buzzer1";

            button.textContent =
              isBuzzer
                ? (
                    newState
                      ? "Desativar"
                      : "Ativar"
                  )
                : (
                    newState
                      ? "Desligar"
                      : "Ligar"
                  );

            if (stateLabel) {
              stateLabel.textContent =
                isBuzzer
                  ? (
                      newState
                        ? "Ativo"
                        : "Inativo"
                    )
                  : (
                      newState
                        ? "Ligado"
                        : "Desligado"
                    );

              stateLabel.classList.toggle(
                "on",
                newState
              );

              stateLabel.classList.toggle(
                "off",
                !newState
              );
            }

            console.log(
              "[RIW] Comando UNO confirmado:",
              capability,
              newState
            );
          }
          catch (error) {
            console.error(
              "[RIW] Falha ao controlar UNO:",
              error
            );

            toast(
              "Arduino UNO",
              error.message ||
              "Falha ao enviar comando."
            );
          }
          finally {
            button.disabled =
              false;
          }
        };
    }
  );
}

function renderMegaCapabilities() {
  
   const defaultGrid =
    qs("#defaultShieldGrid");

  const panel =
    qs("#deviceCapabilitiesPanel");

  if (!panel) {
    return;
  }

  if (
    panel.dataset.device ===
    "TSX-MEGA-001"
  ) {
    return;
  }

  panel.dataset.device =
    "TSX-MEGA-001";

  if (defaultGrid) {
    defaultGrid.style.display =
      "none";
  }

  panel.style.display =
    "block";
 
  panel.innerHTML = `
    <div class="mega-display-card">

      <div class="mega-display-header">
        <div>
          <h3>Display TFT 2.4"</h3>
          <span>
            Envie uma mensagem diretamente
            para o display conectado ao Arduino Mega.
          </span>
        </div>
      </div>

      <div class="mega-display-control">

        <label for="megaDisplayText">
          Mensagem
        </label>

        <div class="mega-display-input-row">

          <input
            type="text"
            id="megaDisplayText"
            class="mega-display-input"
            maxlength="80"
            placeholder="Digite uma mensagem..."
            autocomplete="off"
          >

          <button
            type="button"
            class="btn btn-primary"
            id="megaDisplaySendBtn">
            Enviar ao Display
          </button>

        </div>

      </div>

    </div>

    <div class="mega-expression-card">

  <div class="mega-display-header">
    <div>
      <h3>Expressões Interativas</h3>

      <span>
        Escolha uma expressão para exibir no TFT.
      </span>
    </div>
  </div>

  <div class="mega-expression-grid">

    <button
      type="button"
      class="mega-expression-btn"
      data-expression="happy">

      <span class="mega-expression-icon">
        😀
      </span>

      <span>Feliz</span>

    </button>


    <button
      type="button"
      class="mega-expression-btn"
      data-expression="cool">

      <span class="mega-expression-icon">
        😎
      </span>

      <span>Cool</span>

    </button>


    <button
      type="button"
      class="mega-expression-btn"
      data-expression="love">

      <span class="mega-expression-icon">
        ❤️
      </span>

      <span>Love</span>

    </button>


    <button
      type="button"
      class="mega-expression-btn"
      data-expression="surprise">

      <span class="mega-expression-icon">
        😮
      </span>

      <span>Surpresa</span>

    </button>

  </div>

  <div
    class="mega-expression-status"
    id="megaExpressionStatus">

    Selecione uma expressão.

  </div>

  </div>

   <div class="mega-interaction-card">

  <div class="mega-interaction-header">
    <div>
      <span class="mega-photo-kicker">
        C++ EXPERIENCE
      </span>

      <h3>
        Interação do Visitante
      </h3>

      <p>
        Assine ou registre uma foto para participar
        do mural ao vivo do evento.
      </p>
    </div>
  </div>

  <div class="mega-interaction-grid">


   <div class="mega-signature-card">

  <div class="mega-display-header">
    <div>
      <h3>Assinador TFT</h3>

      <span>
        Desenhe sua assinatura e envie para o display.
      </span>
    </div>
  </div>

  <canvas
    id="megaSignatureCanvas"
    width="320"
    height="240">
  </canvas>

  <div class="mega-signature-actions">

    <button
      type="button"
      class="btn"
      id="megaSignatureClear">
      Limpar
    </button>

    <button
      type="button"
      class="btn btn-primary"
      id="megaSignatureSend">
      Enviar para TFT
    </button>

  </div>

 </div>

  <div class="mega-photo-card">

  <div class="mega-photo-header">

    <div>
      <span class="mega-photo-kicker">
        C++ EXPERIENCE
      </span>

      <h3>
        Foto Experience
      </h3>

      <p>
        Registre seu momento no evento
        e participe do mural ao vivo.
      </p>
    </div>

    <div class="mega-photo-camera-icon">
      📸
    </div>

  </div>


  <div class="mega-photo-machine">

    <div class="mega-photo-machine-top">

      <span class="mega-photo-live-dot"></span>

      <span>
        Câmera pronta
      </span>

    </div>

      <div class="mega-photo-device-row">

  <label for="megaPhotoCameraSelect">
    Câmera
  </label>

  <select
    id="megaPhotoCameraSelect"
    class="mega-photo-camera-select"
  >
    <option value="">
      Detectando câmeras...
    </option>
  </select>

</div>


    <div
      class="mega-photo-preview"
      id="megaPhotoPreview"
    >

      <video
         id="megaPhotoVideo"
         class="mega-photo-video"
         autoplay
         playsinline
         muted
      ></video>

        <canvas
  id="megaPhotoCanvas"
  class="mega-photo-canvas"
></canvas>

<div
  id="megaPhotoCountdown"
  class="mega-photo-countdown"
></div>

<div
  id="megaPhotoFlash"
  class="mega-photo-flash"
></div>

      <div
        class="mega-photo-placeholder"
        id="megaPhotoPlaceholder"
      >
         <div class="mega-photo-lens">
            <div class="mega-photo-lens-inner">
          </div>
         </div>

        <strong>
          Prepare-se para a foto
        </strong>

        <span>
          A câmera será exibida aqui
        </span>

      </div>

    </div>


    <div class="mega-photo-controls">

    <button
    type="button"
    class="mega-photo-capture-btn"
    id="megaPhotoCaptureBtn"
   >
    <span class="mega-photo-capture-icon">
      ●
    </span>

    Tirar Foto
   </button>


   <button
    type="button"
    class="mega-photo-save-btn"
    id="megaPhotoSaveBtn"
    style="display: none;"
   >
    ✓ Salvar Foto
    </button>

     </div>
        <span class="mega-photo-capture-icon">
          ●
        </span>

        Tirar Foto
      </button>

    </div>


    <div
      class="mega-photo-status"
      id="megaPhotoStatus"
    >
      Posicione-se em frente à câmera.
    </div>

  </div>

</div>

`;

  bindMegaDisplayControl();
  bindMegaExpressionControls();
  bindMegaSignatureControl();
  bindMegaPhotoControl();
}

function bindMegaSignatureControl() {
  const canvas =
    document.getElementById(
      "megaSignatureCanvas"
    );

  const clearButton =
    document.getElementById(
      "megaSignatureClear"
    );

  const sendButton =
    document.getElementById(
      "megaSignatureSend"
    );

  if (!canvas) {
    return;
  }

  const ctx =
    canvas.getContext("2d");

  ctx.strokeStyle =
    "#ffffff";

  ctx.lineWidth =
    3;

  ctx.lineCap =
    "round";

  ctx.lineJoin =
    "round";

  canvas.style.pointerEvents =
    "none";


  let lastRenderedCount =
    0;

  let pollingActive =
    false;

  let pollingTimer =
    null;

  // Importante:
  // só consideramos a sessão realmente iniciada
  // depois que chegaram traços do TFT.
  let signatureStarted =
    false;


  // ======================================================
  // LIMPAR
  // ======================================================

  const clearCanvas =
    () => {
      ctx.clearRect(
        0,
        0,
        canvas.width,
        canvas.height
      );

      lastRenderedCount =
        0;

      signatureStarted =
        false;
    };


  // ======================================================
  // DESENHAR SEGMENTO
  // ======================================================

  const drawSegment =
    segment => {
      ctx.beginPath();

      ctx.moveTo(
        segment.x1,
        segment.y1
      );

      ctx.lineTo(
        segment.x2,
        segment.y2
      );

      ctx.stroke();
    };


  // ======================================================
  // PARAR POLLING
  // ======================================================

  const stopPolling =
    () => {
      pollingActive =
        false;

      if (pollingTimer) {
        clearTimeout(
          pollingTimer
        );

        pollingTimer =
          null;
      }
    };


  // ======================================================
  // POLLING
  // ======================================================

  const pollSignature =
    async () => {
      if (!pollingActive) {
        return;
      }

      try {
        const response =
          await fetch(
            "/api/mega/signature/live",
            {
              method: "GET",
              cache: "no-store"
            }
          );

        const result =
          await response.json();

        if (!response.ok) {
          throw new Error(
            result?.error ||
            `HTTP ${response.status}`
          );
        }

        const segments =
          Array.isArray(
            result.segments
          )
            ? result.segments
            : [];


        // --------------------------------------------------
        // CHEGARAM NOVOS TRAÇOS
        // --------------------------------------------------

        if (
          segments.length >
          lastRenderedCount
        ) {
          signatureStarted =
            true;
        }


        for (
          let i =
            lastRenderedCount;

          i < segments.length;

          i++
        ) {
          drawSegment(
            segments[i]
          );
        }


        lastRenderedCount =
          segments.length;


        // ==================================================
        // ASSINATURA REALMENTE FINALIZADA
        // ==================================================
        //
        // Não basta active=false.
        // Precisamos ter recebido assinatura antes.
        //
        // Isso evita encerrar o polling cedo demais.
        // ==================================================

        if (
          !result.active &&
          signatureStarted
        ) {
          stopPolling();

          if (sendButton) {
            sendButton.disabled =
              true;

            sendButton.textContent =
              "Assinatura gravada";
          }

          console.log(
            "[RIW] Assinatura concluída."
          );


          // ------------------------------------------------
          // Mantém assinatura visível brevemente.
          // Depois prepara para próximo participante.
          // ------------------------------------------------

          setTimeout(
            () => {
              clearCanvas();

              if (sendButton) {
                sendButton.disabled =
                  false;

                sendButton.textContent =
                  "Iniciar assinatura";
              }

              console.log(
                "[RIW] Assinador pronto para o próximo participante."
              );
            },
            1800
          );

          return;
        }
      }
      catch (error) {
        console.error(
          "[RIW] Falha ao consultar assinatura:",
          error
        );
      }


      // Continua acompanhando o TFT.
      if (pollingActive) {
        pollingTimer =
          setTimeout(
            pollSignature,
            150
          );
      }
    };


  // ======================================================
  // INICIAR ASSINATURA
  // ======================================================

  const startSignature =
    async () => {
      if (!sendButton) {
        return;
      }

      stopPolling();

      clearCanvas();

      signatureStarted =
        false;

      sendButton.disabled =
        true;

      sendButton.textContent =
        "Aguardando assinatura...";


      try {
        const response =
          await fetch(
            "/api/mega/signature/start",
            {
              method: "POST"
            }
          );

        const result =
          await response.json();


        if (!response.ok) {
          throw new Error(
            result?.error ||
            `HTTP ${response.status}`
          );
        }


        pollingActive =
          true;

        pollSignature();
      }
      catch (error) {
        console.error(
          "[RIW] Falha ao iniciar assinatura:",
          error
        );

        sendButton.disabled =
          false;

        sendButton.textContent =
          "Erro ao iniciar";


        setTimeout(
          () => {
            sendButton.textContent =
              "Iniciar assinatura";
          },
          1500
        );
      }
    };


  // ======================================================
  // BOTÃO LIMPAR
  // ======================================================

  if (clearButton) {
    clearButton.addEventListener(
      "click",
      () => {
        clearCanvas();
      }
    );
  }


  // ======================================================
  // BOTÃO INICIAR
  // ======================================================

  if (sendButton) {
    sendButton.textContent =
      "Iniciar assinatura";

    sendButton.addEventListener(
      "click",
      startSignature
    );
  }
}

async function bindMegaPhotoControl() {
  const video =
    qs("#megaPhotoVideo");

  const placeholder =
    qs("#megaPhotoPlaceholder");

  const status =
    qs("#megaPhotoStatus");

  const button =
    qs("#megaPhotoCaptureBtn");

  const saveButton =
    qs("#megaPhotoSaveBtn");

  const cameraSelect =
    qs("#megaPhotoCameraSelect");

  const canvas =
    qs("#megaPhotoCanvas");

  const countdown =
    qs("#megaPhotoCountdown");

  const flash =
    qs("#megaPhotoFlash");


  if (
    !video ||
    !button ||
    !cameraSelect
  ) {
    return;
  }


  // ======================================================
  // SEGURANÇA / SUPORTE DO NAVEGADOR
  // ======================================================

  if (!window.isSecureContext) {

  // ------------------------------------------------------
  // MOBILE / HTTP
  // A câmera via getUserMedia exige contexto seguro.
  // Mantemos todos os demais recursos funcionando.
  // ------------------------------------------------------

  if (video) {
    video.style.display =
      "none";
  }

  if (canvas) {
    canvas.style.display =
      "none";
  }

  if (cameraSelect) {
    cameraSelect.style.display =
      "none";
  }

  if (button) {
    button.style.display =
      "none";
  }

  if (saveButton) {
    saveButton.style.display =
      "none";
  }

  if (countdown) {
    countdown.style.display =
      "none";
  }

  if (flash) {
    flash.classList.remove(
      "active"
    );
  }

  if (placeholder) {
    placeholder.style.display =
      "flex";

    placeholder.innerHTML = `
      <div class="mega-photo-camera-icon">
        📱
      </div>

      <strong>
        Foto disponível no terminal do evento
      </strong>

      <span>
        A câmera do celular não está disponível
        nesta conexão local.
      </span>

      <small>
        Utilize o computador principal
        do C++ Experience para registrar sua foto.
      </small>
    `;
  }

  if (status) {
    status.textContent =
      "Os demais recursos continuam disponíveis normalmente.";
  }

  console.log(
    "[RIW] Foto Experience em modo HTTP sem câmera:",
    window.location.href
  );

  return;
}


  if (
    !navigator.mediaDevices ||
    !navigator.mediaDevices.getUserMedia
  ) {
    if (status) {
      status.textContent =
        "Câmera não suportada pelo navegador.";
    }

    return;
  }


  // ======================================================
  // STREAM ATUAL
  // ======================================================

  let currentStream =
    null;


  const stopCurrentStream =
    () => {
      if (!currentStream) {
        return;
      }

      currentStream
        .getTracks()
        .forEach(
          track => track.stop()
        );

      currentStream =
        null;
    };


  // ======================================================
  // CARREGAR LISTA DE CÂMERAS
  // ======================================================

  const loadCameraList =
    async () => {
      const devices =
        await navigator.mediaDevices
          .enumerateDevices();

      const cameras =
        devices.filter(
          device =>
            device.kind ===
            "videoinput"
        );

      cameraSelect.innerHTML =
        "";

      cameras.forEach(
        (camera, index) => {
          const option =
            document.createElement(
              "option"
            );

          option.value =
            camera.deviceId;

          option.textContent =
            camera.label ||
            `Câmera ${index + 1}`;

          cameraSelect.appendChild(
            option
          );
        }
      );

      return cameras;
    };


  // ======================================================
  // INICIAR CÂMERA
  // ======================================================

  const startCamera =
    async (
      deviceId = ""
    ) => {
      try {
        stopCurrentStream();

        if (status) {
          status.textContent =
            "Iniciando câmera...";
        }

        const constraints = {
          video: deviceId
            ? {
                deviceId: {
                  exact: deviceId
                },

                width: {
                  ideal: 1280
                },

                height: {
                  ideal: 720
                }
              }
            : {
                width: {
                  ideal: 1280
                },

                height: {
                  ideal: 720
                },

                facingMode:
                  "user"
              },

          audio: false
        };


        currentStream =
          await navigator.mediaDevices
            .getUserMedia(
              constraints
            );


        video.srcObject =
          currentStream;

        await video.play();

        video.style.display =
          "block";


        if (canvas) {
          canvas.style.display =
            "none";
        }


        if (placeholder) {
          placeholder.style.display =
            "none";
        }


        const track =
          currentStream
            .getVideoTracks()[0];


        if (track) {
          const settings =
            track.getSettings();

          if (settings.deviceId) {
            cameraSelect.value =
              settings.deviceId;
          }

          console.log(
            "[RIW] Câmera ativa:",
            track.label
          );
        }


        if (status) {
          status.textContent =
            "Câmera ativa. Posicione-se para a foto.";
        }
      }
      catch (error) {
        console.error(
          "[RIW] Falha ao acessar câmera:",
          error
        );

        if (status) {
          status.textContent =
            "Não foi possível acessar a câmera selecionada.";
        }

        if (placeholder) {
          placeholder.style.display =
            "flex";
        }

        video.style.display =
          "none";
      }
    };


  // ======================================================
  // INICIALIZAÇÃO
  // ======================================================

  try {
    await startCamera();

    await loadCameraList();

    const activeTrack =
      currentStream
        ?.getVideoTracks()[0];

    const activeSettings =
      activeTrack
        ?.getSettings();


    if (activeSettings?.deviceId) {
      cameraSelect.value =
        activeSettings.deviceId;
    }
  }
  catch (error) {
    console.error(
      "[RIW] Erro ao inicializar câmera:",
      error
    );
  }


  // ======================================================
  // TROCAR CÂMERA
  // ======================================================

  cameraSelect.addEventListener(
    "change",
    async () => {
      const deviceId =
        cameraSelect.value;

      if (!deviceId) {
        return;
      }

      await startCamera(
        deviceId
      );
    }
  );


  // ======================================================
  // TIRAR / REFAZER FOTO
  // ======================================================

  button.addEventListener(
    "click",
    async () => {
      if (
        !video ||
        !canvas ||
        !countdown ||
        !flash
      ) {
        return;
      }


      // --------------------------------------------------
      // REFAZER FOTO
      // --------------------------------------------------

      if (
        button.textContent.trim() ===
        "Refazer Foto"
      ) {
        canvas.style.display =
          "none";

        video.style.display =
          "block";

        try {
          await video.play();
        }
        catch (error) {
          console.error(
            "[RIW] Erro ao retornar para câmera:",
            error
          );
        }

        button.textContent =
          "Tirar Foto";


        if (saveButton) {
          saveButton.style.display =
            "none";

          saveButton.textContent =
            "✓ Salvar Foto";
        }


        if (status) {
          status.textContent =
            "Câmera ativa. Posicione-se para a foto.";
        }

        return;
      }


      // --------------------------------------------------
      // CONTAGEM REGRESSIVA
      // --------------------------------------------------

      button.disabled =
        true;


      if (status) {
        status.textContent =
          "Prepare-se...";
      }


      for (
        let value = 3;
        value >= 1;
        value--
      ) {
        countdown.textContent =
          String(value);

        countdown.style.display =
          "flex";

        await new Promise(
          resolve =>
            setTimeout(
              resolve,
              1000
            )
        );
      }


      countdown.style.display =
        "none";


      // --------------------------------------------------
      // FLASH
      // --------------------------------------------------

      flash.classList.remove(
        "active"
      );

      void flash.offsetWidth;

      flash.classList.add(
        "active"
      );


      // --------------------------------------------------
      // CAPTURAR FRAME
      // --------------------------------------------------

      const context =
        canvas.getContext(
          "2d"
        );

      canvas.width =
        video.videoWidth;

      canvas.height =
        video.videoHeight;


      context.drawImage(
        video,
        0,
        0,
        canvas.width,
        canvas.height
      );


      canvas.style.display =
        "block";

      video.style.display =
        "none";


      if (status) {
        status.textContent =
          "Foto capturada.";
      }


      button.disabled =
        false;

      button.textContent =
        "Refazer Foto";


      if (saveButton) {
        saveButton.style.display =
          "inline-flex";
      }
    }
  );


  // ======================================================
  // SALVAR FOTO
  // ======================================================

  saveButton?.addEventListener(
    "click",
    async () => {
      if (
        !canvas ||
        canvas.width === 0 ||
        canvas.height === 0
      ) {
        return;
      }


      saveButton.disabled =
        true;

      saveButton.textContent =
        "Salvando...";


      if (status) {
        status.textContent =
          "Salvando foto no mural...";
      }


      try {
        const image =
          canvas.toDataURL(
            "image/jpeg",
            0.88
          );


        const eventDate =
          gallerySelectedDate ||
          getDefaultGalleryDate();


        const response =
          await fetch(
            "/api/gallery/photos",
            {
              method: "POST",

              headers: {
                "Content-Type":
                  "application/json"
              },

              body:
                JSON.stringify({
                  date:
                    eventDate,

                  image:
                    image
                })
            }
          );


        const result =
          await response.json();


        if (!response.ok) {
          throw new Error(
            result?.error ||
            `HTTP ${response.status}`
          );
        }


        console.log(
          "[RIW] Foto salva:",
          result
        );


        saveButton.textContent =
          "Foto Salva";


        if (status) {
          status.textContent =
            "Foto adicionada ao Mural C++ Experience.";
        }


        // ==================================================
        // PREPARAR PARA O PRÓXIMO PARTICIPANTE
        // ==================================================

   setTimeout(
     async () => {
      try {
      console.log(
        "[RIW] Reiniciando Foto Experience..."
      );

      // --------------------------------------------
      // Esconde e limpa a foto capturada
      // --------------------------------------------

      canvas.style.display =
        "none";

      const context =
        canvas.getContext(
          "2d"
        );

      if (context) {
        context.clearRect(
          0,
          0,
          canvas.width,
          canvas.height
        );
      }

      // --------------------------------------------
      // Limpa efeitos da captura
      // --------------------------------------------

      countdown.style.display =
        "none";

      countdown.textContent =
        "";

      flash.classList.remove(
        "active"
      );

      // --------------------------------------------
      // Restaura os botões
      // --------------------------------------------

      button.disabled =
        false;

      button.textContent =
        "Tirar Foto";

      saveButton.style.display =
        "none";

      saveButton.disabled =
        false;

      saveButton.textContent =
        "✓ Salvar Foto";

      // --------------------------------------------
      // Reinicia a câmera selecionada
      // --------------------------------------------

      const selectedCamera =
        cameraSelect.value;

      await startCamera(
        selectedCamera
      );

      if (status) {
        status.textContent =
          "Câmera ativa. Posicione-se para a foto.";
      }

      console.log(
        "[RIW] Foto Experience pronta para o próximo participante"
      );
    }
    catch (error) {
      console.error(
        "[RIW] Falha ao reiniciar Foto Experience:",
        error
      );
    }
  },
  1800
);
      }
      catch (error) {
        console.error(
          "[RIW] Erro ao salvar foto:",
          error
        );


        saveButton.textContent =
          "Erro ao salvar";


        if (status) {
          status.textContent =
            "Não foi possível salvar a foto.";
        }
      }
      finally {
        saveButton.disabled =
          false;
      }
    }
  );
}

function bindMegaDisplayControl() {
  const input =
    qs("#megaDisplayText");

  const button =
    qs("#megaDisplaySendBtn");

  if (
    !input ||
    !button
  ) {
    return;
  }

  const sendMessage =
    async () => {
      const originalText =
        input.value.trim();

      const text =
         originalText
         .normalize("NFD")
         .replace(/[\u0300-\u036f]/g, "");

      if (!text) {
        input.focus();
        return;
      }

      button.disabled = true;
      button.textContent =
        "Enviando...";

      try {
        const response =
          await fetch(
            "/api/mega/display",
            {
              method: "POST",

              headers: {
                "Content-Type":
                  "application/json",
              },

              body:
                JSON.stringify({
                  text: text,
                }),
            },
          );

        const result =
          await response.json();

        if (!response.ok) {
          throw new Error(
            result?.error ||
            `HTTP ${response.status}`,
          );
        }

        console.log(
          "[RIW] Mensagem enviada ao TFT:",
          text,
        );

        button.textContent =
          "Enviado";

        setTimeout(
          () => {
            button.textContent =
              "Enviar ao Display";
          },
          1200,
        );
      }
      catch (error) {
        console.error(
          "[RIW] Falha ao enviar mensagem ao TFT:",
          error,
        );

        button.textContent =
          "Erro ao enviar";

        setTimeout(
          () => {
            button.textContent =
              "Enviar ao Display";
          },
          1500,
        );
      }
      finally {
        button.disabled =
          false;
      }
    };

  button.addEventListener(
    "click",
    sendMessage,
  );

  input.addEventListener(
    "keydown",
    (event) => {
      if (
        event.key === "Enter"
      ) {
        event.preventDefault();

        sendMessage();
      }
    },
  );
}

function bindMegaExpressionControls() {
  const buttons =
    document.querySelectorAll(
      ".mega-expression-btn"
    );

  const status =
    qs("#megaExpressionStatus");

  buttons.forEach(
    (button) => {

      button.addEventListener(
        "click",
        async () => {

          const expression =
            button.dataset.expression;

          if (!expression) {
            return;
          }

          buttons.forEach(
            item => {
              item.disabled = true;
            }
          );

          if (status) {
            status.textContent =
              "Enviando ao TFT...";
          }

          try {
            const response =
              await fetch(
                "/api/mega/expression",
                {
                  method: "POST",

                  headers: {
                    "Content-Type":
                      "application/json",
                  },

                  body:
                    JSON.stringify({
                      expression:
                        expression,
                    }),
                }
              );

            const result =
              await response.json();

            if (!response.ok) {
              throw new Error(
                result?.error ||
                `HTTP ${response.status}`
              );
            }

            buttons.forEach(
              item => {
                item.classList.remove(
                  "active"
                );
              }
            );

            button.classList.add(
              "active"
            );

            if (status) {
              status.textContent =
                `Expressão exibida: ${
                  button.innerText.trim()
                }`;
            }
          }
          catch (error) {
            console.error(
              "[RIW] Falha ao enviar expressão:",
              error
            );

            if (status) {
              status.textContent =
                "Erro ao enviar expressão.";
            }
          }
          finally {
            buttons.forEach(
              item => {
                item.disabled =
                  false;
              }
            );
          }
        }
      );
    }
  );
}

function updateActiveDevicesCard(data) {
  const countElement =
    qs("#activeDevicesCount");

  const labelElement =
    qs("#activeDevicesLabel");

  if (
    !countElement ||
    !labelElement
  ) {
    console.warn(
      "[RIW] Card de dispositivos ativos não localizado"
    );

    return;
  }

  // Sem dispositivo
  if (
    !data ||
    data.connected !== true
  ) {
    countElement.textContent =
      "0";

    labelElement.textContent =
      "Nenhum dispositivo";

    return;
  }

  // Dispositivo conectado
  countElement.textContent =
    "1";

  const deviceName =
    data.device?.name ||
    "Dispositivo conectado";

  labelElement.textContent =
    deviceName;

  console.log(
    "[RIW] Dispositivo ativo:",
    deviceName
  );
}

async function loadConnectedDevices() {
  try {
    const response =
      await fetch("/api/devices");

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }

    const data =
      await response.json();

    updateConnectedDevicesUI(data);
  }
  catch (error) {
    console.error(
      "[RIW] Erro ao carregar dispositivos:",
      error
    );

    updateConnectedDevicesUI({
      count: 0,
      devices: []
    });
  }
}

function updateConnectedDevicesUI(data) {
  const countElement =
    document.querySelector(
      "#activeDevicesCount"
    );

  const labelElement =
    document.querySelector(
      "#activeDevicesLabel"
    );

  const devices =
    Array.isArray(data?.devices)
      ? data.devices
      : [];
    state.devices = devices;
    
    // ======================================================
// RESUMO REAL DO SISTEMA
// ======================================================

const summaryDevices =
  qs("#summaryDevices");

const summaryCommunication =
  qs("#summaryCommunication");

const errorCount =
  qs("#errorCount");


const totalDevices =
  devices.length;

const onlineDevices =
  devices.filter(
    device =>
      device.online !== false
  ).length;


if (summaryDevices) {
  summaryDevices.textContent =
    `${onlineDevices}/${totalDevices}`;
}


if (summaryCommunication) {

  if (totalDevices === 0) {
    summaryCommunication.textContent =
      "Aguardando";
  }
  else if (
    onlineDevices === totalDevices
  ) {
    summaryCommunication.textContent =
      "OK";
  }
  else {
    summaryCommunication.textContent =
      "Parcial";
  }
}


if (errorCount) {

  const errors =
    Math.max(
      0,
      totalDevices -
      onlineDevices
    );

  errorCount.textContent =
    String(errors);
}
    
    // ======================================================
// STATUS GLOBAL DA CONEXÃO
// Baseado na API multi-device /api/devices
// ======================================================

const connectionLabel =
  qs("#connectionLabel");

const statusDot =
  qs(
    ".connection-status .status-dot"
  );

const portSelect =
  qs("#portSelect");


if (devices.length === 0) {

  if (connectionLabel) {
    connectionLabel.textContent =
      "Aguardando placa";
  }

  if (statusDot) {
    statusDot.style.background =
      "#ffb020";

    statusDot.style.boxShadow =
      "0 0 12px rgba(255,176,32,.65)";
  }

  if (portSelect) {
    portSelect.innerHTML =
      '<option value="">Nenhuma porta conectada</option>';
  }
}
else {

  if (connectionLabel) {
    connectionLabel.textContent =
      devices.length === 1
        ? "1 placa conectada"
        : `${devices.length} placas conectadas`;
  }

  if (statusDot) {
    statusDot.style.background =
      "#29e882";

    statusDot.style.boxShadow =
      "0 0 12px rgba(41,232,130,.75)";
  }

  if (portSelect) {

    portSelect.innerHTML =
      devices
        .map(
          device => `
            <option value="${device.port}">
              ${device.name} - ${device.port}
            </option>
          `
        )
        .join("");
  }
  } 

  if (countElement) {
    countElement.textContent =
      String(devices.length);
  }

  if (!labelElement) {
    return;
  }

  if (devices.length === 0) {
    labelElement.textContent =
      "Nenhum dispositivo";

    return;
  }

  labelElement.textContent =
    devices
      .map(device => {
        if (
          device.id ===
          "TSX-UNO-001"
        ) {
          return "UNO";
        }

        if (
          device.id ===
          "TSX-MEGA-001"
        ) {
          return "MEGA";
        }

        if (
          device.id ===
          "TSX-ESP32-001"
        ) {
          return "ESP32";
        }

        return device.name;
      })
      .join(" · ");

     renderDashboardDevices(
     devices
    ); 
      
     if (
     state.currentView === "shields" &&
     state.shieldsMode === "devices"
     ) {
    
     renderShieldsDeviceSelector(devices);
}
    // ======================================================
// ATUALIZAÇÃO EM TEMPO REAL DA ABA DISPOSITIVOS
// ======================================================

if (
  state.currentView ===
  "devices"
) {
  renderDevicesSelector(
    devices
  );
}

}

function renderDashboardDevices(devices) {
  const grid =
    qs("#dashboardDeviceGrid");

  const count =
    qs("#dashboardDeviceCount");

  if (!grid) {
    return;
  }

  const list =
    Array.isArray(devices)
      ? devices
      : [];

  if (count) {
    count.textContent =
      `${list.length} ${
        list.length === 1
          ? "dispositivo"
          : "dispositivos"
      }`;
  }

  grid.innerHTML = "";

  if (list.length === 0) {
    grid.innerHTML = `
      <div class="dashboard-empty">
        Nenhuma placa conectada
      </div>
    `;

    return;
  }

  list.forEach(device => {
    const image =
      getDeviceImage(
        device.id
      );

    const card =
      document.createElement(
        "article"
      );

    card.className =
      "dashboard-device-card";

    card.dataset.deviceId =
      device.id;

       card.innerHTML = `
  <div class="shield-device-layout">

    <div class="shield-device-text">

      <h3>${device.name}</h3>

      <div class="shield-device-id">
        ${device.id}
      </div>

      <div class="shield-device-status-row">

        <span class="shield-device-online">
          Online
        </span>

        <span class="shield-device-port">
          ${device.port}
        </span>

      </div>

      <button
        type="button"
        class="btn btn-primary device-resources-btn">
        Ver Recursos
      </button>

    </div>

    <div class="shield-device-image ${
  device.id === "TSX-UNO-001"
    ? "dashboard-image-uno"
    : device.id === "TSX-ESP32-001"
      ? "dashboard-image-esp32"
      : "dashboard-image-default"
}">
      <img
        src="${getDeviceImage(device.id)}"
        alt="${device.name}">
    </div>

    </div>

    `;

    grid.appendChild(card);

    card
  .querySelector(
    ".device-resources-btn"
  )
  ?.addEventListener(
    "click",
    () => {
      // Muda a aplicação para Shields.
      state.currentView =
        "shields";

      state.shieldsMode =
        "resources";

      state.selectedShieldDeviceId =
        device.id;

      // Atualiza visualmente o menu lateral.
      qsa(".nav-item").forEach(
        item => {
          item.classList.toggle(
            "active",
            item.textContent.includes(
              "Shields"
            )
          );
        }
      );

      // Abre diretamente os recursos
      // da placa selecionada.
      selectDeviceForShields(
        device
      );
    }
  );

  });
}

function getDeviceImage(deviceId) {
  switch (deviceId) {

    case "TSX-UNO-001":
      return "assets/arduino-uno.png";

    case "TSX-MEGA-001":
      return "assets/arduino-mega.png";

    case "TSX-ESP32-001":
      return "assets/esp32s3.png";

    default:
      return "assets/device-generic.png";
  }
}

function showDashboardView() {
  
   const about =
  qs("#aboutSection");

  if (about) {
  about.style.display =
    "none";
  }

  state.currentView =
    "dashboard";

  const dashboard =
    qs("#dashboardDevicesSection");

  const capabilities =
    qs("#deviceCapabilitiesPanel");

  const shieldsTitle =
    qs("#connectedShieldsTitle");

  const gallery =
    qs("#gallerySection");

  const metrics =
  qs(".metrics-grid");

  if (metrics) {
  metrics.style.display =
    "";
  }  

  if (dashboard) {
    dashboard.style.display =
      "block";
  }

  if (capabilities) {
    capabilities.style.display =
      "none";

    capabilities.innerHTML =
      "";
  }

  if (shieldsTitle) {
    shieldsTitle.style.display =
      "none";
  }

  if (gallery) {
    gallery.style.display =
      "none";
  }

  renderDashboardDevices(
    state.devices
  );
}


function showShieldsView() {
   
   const about =
  qs("#aboutSection");

   if (about) {
  about.style.display =
    "none";
  }

  state.currentView = "shields";
  state.shieldsMode = "devices";
  state.selectedShieldDeviceId = null;

  const dashboard =
    qs("#dashboardDevicesSection");

  const capabilities =
    qs("#deviceCapabilitiesPanel");

  const shieldsTitle =
    qs("#connectedShieldsTitle");

  const gallery =
    qs("#gallerySection");  

  const metrics =
  qs(".metrics-grid");

  if (metrics) {
  metrics.style.display =
    "";
  }  

  if (dashboard) {
    dashboard.style.display =
      "none";
  }

  if (capabilities) {
    capabilities.style.display =
      "grid";
  }

  if (shieldsTitle) {
    shieldsTitle.style.display =
      "";
  }

  if (gallery) {
    gallery.style.display =
    "none";
  }

  renderShieldsDeviceSelector(
    state.devices
  );
}

function showAboutView() {
  state.currentView =
    "about";

  const dashboard =
    qs(
      "#dashboardDevicesSection"
    );

  const capabilities =
    qs(
      "#deviceCapabilitiesPanel"
    );

  const shieldsTitle =
    qs(
      "#connectedShieldsTitle"
    );

  const gallery =
    qs(
      "#gallerySection"
    );

  const about =
    qs(
      "#aboutSection"
    );

    const metrics =
    qs(".metrics-grid");

   stopGalleryAutoRefresh();


  if (dashboard) {
    dashboard.style.display =
      "none";
  }


  if (capabilities) {
    capabilities.style.display =
      "none";
  }


  if (shieldsTitle) {
    shieldsTitle.style.display =
      "none";
  }

  if (metrics) {
    metrics.style.display =
      "none";
  }


  if (gallery) {
    gallery.style.display =
      "none";
  }


  if (about) {
    about.style.display =
      "block";
  }
}


function showGalleryView() {
   const about =
  qs("#aboutSection");

  if (about) {
  about.style.display =
    "none";
  }

  state.currentView = "gallery";
  
   document.body.classList.add(
   "gallery-mode"
    );

   const gallery =
    qs("#gallerySection");

   if (
    gallery &&
    gallery.parentElement !==
    document.body
    ) {
    document.body.appendChild(
    gallery
    );
  } 
   
  const layout =
  qs(".layout");

  const sidebar =
  qs(".sidebar");

  const mainHeader =
  qs(".topbar");  

  const dashboard =
    qs("#dashboardDevicesSection");

  const capabilities =
    qs("#deviceCapabilitiesPanel");

  const shieldsTitle =
    qs("#connectedShieldsTitle");

  const metrics =
  qs(".metrics-grid");

  if (metrics) {
  metrics.style.display =
    "";
  }  

  if (dashboard) {
    dashboard.style.display =
      "none";
  }

  if (capabilities) {
    capabilities.style.display =
      "none";
  }

  if (shieldsTitle) {
    shieldsTitle.style.display =
      "none";
  }

  if (sidebar) {
  sidebar.style.display =
    "none";
  }

  if (mainHeader) {
   mainHeader.style.display =
    "none";
  }

  if (layout) {
  layout.style.display =
    "block";
  }

  if (gallery) {
    gallery.style.display =
      "block";
  }

  if (!gallerySelectedDate) {
  gallerySelectedDate =
    getDefaultGalleryDate();
}

updateGalleryDaySelection();

loadGallerySignatures(
  gallerySelectedDate
);

loadGalleryPhotos(
  gallerySelectedDate
);

startGalleryAutoRefresh();


}

function showDevicesView() {
   
  const about =
  qs("#aboutSection");

  if (about) {
  about.style.display =
    "none";
  }
  state.currentView = "devices";

  const dashboard =
    qs("#dashboardDevicesSection");

  const capabilities =
    qs("#deviceCapabilitiesPanel");

  const shieldsTitle =
    qs("#connectedShieldsTitle");

  const gallery =
    qs("#gallerySection"); 

  const metrics =
  qs(".metrics-grid");

  if (metrics) {
  metrics.style.display =
    "";
  }   

  if (gallery) {
  gallery.style.display =
    "none";
  }  

  if (dashboard) {
    dashboard.style.display = "none";
  }

  if (capabilities) {
  capabilities.style.display =
    "grid";

  capabilities.style.gridTemplateColumns =
    "1fr";
}

  if (shieldsTitle) {
    shieldsTitle.style.display = "";
    shieldsTitle.textContent =
      "DISPOSITIVOS CONECTADOS";
  }

  renderDevicesSelector(
    state.devices
  );
}

function showSerialMonitorView() {
  const about =
  qs("#aboutSection");

  if (about) {
  about.style.display =
    "none";
  } 

  state.currentView = "serial";

  const dashboard =
    qs("#dashboardDevicesSection");

  const panel =
    qs("#deviceCapabilitiesPanel");

  const title =
    qs("#connectedShieldsTitle");

  const gallery =
    qs("#gallerySection");  

  const metrics =
  qs(".metrics-grid");

  if (metrics) {
  metrics.style.display =
    "";
  }  

  if (gallery) {
     gallery.style.display =
    "none";
  }  

  if (dashboard) {
    dashboard.style.display =
      "none";
  }

  if (panel) {
    panel.style.display =
      "block";

    panel.style.gridTemplateColumns =
      "";

    panel.innerHTML = "";
  }

  if (title) {
    title.style.display =
      "";

    title.textContent =
      "MONITOR SERIAL";
  }

  renderSerialMonitor();
  loadSerialMonitor();
}

function renderSerialMonitor() {
  const panel =
    qs("#deviceCapabilitiesPanel");

  if (!panel) {
    return;
  }

  panel.innerHTML = `
    <section class="serial-monitor">

      <div class="serial-monitor-header">

        <div>
          <h3>Comunicação Serial</h3>

          <p>
            Acompanhe em tempo real a comunicação
            entre o C++Builder e os dispositivos conectados.
          </p>
        </div>

        <div class="serial-monitor-status">
          <span class="serial-live-dot"></span>
          Monitor ativo
        </div>

      </div>


      <div class="serial-device-bar">

        <div class="serial-device-info">
          <span>Dispositivos conectados</span>

          <strong>
            ${state.devices.length}
          </strong>
        </div>

        <div class="serial-device-list">

  <button
    type="button"
    class="
      serial-device-chip
      ${
        serialMonitorFilter === "ALL"
          ? "active"
          : ""
      }
    "
    data-serial-filter="ALL"
  >
    Todos
    <small>
      ${state.devices.length}
    </small>
  </button>

  ${
    state.devices
      .map(
        device => `
          <button
            type="button"
            class="
              serial-device-chip
              ${
                serialMonitorFilter === device.id
                  ? "active"
                  : ""
              }
            "
            data-serial-filter="${device.id}"
          >
            ${device.name}

            <small>
              ${device.port}
            </small>
          </button>
        `
      )
      .join("")
  }

</div>

      </div>


      <div class="serial-terminal">

        <div class="serial-terminal-toolbar">

          <span>Terminal</span>

          <div>
            <button
              type="button"
              class="serial-toolbar-btn"
              id="serialClearBtn">
              Limpar
            </button>
          </div>

        </div>

        <div
          class="serial-terminal-content"
          id="serialTerminalContent">

          <div class="serial-line system">
            <span class="serial-time">
              --
            </span>

            <span class="serial-direction">
              SYS
            </span>

            <span class="serial-text">
              Monitor serial iniciado.
            </span>
          </div>

        </div>

      </div>


      <div class="serial-command-panel">

        <select
          id="serialDeviceSelect"
          class="serial-device-select">

          ${
            state.devices
              .map(device => `
                <option value="${device.id}">
                  ${device.name} - ${device.port}
                </option>
              `)
              .join("")
          }

        </select>

        <input
          type="text"
          id="serialCommandInput"
          placeholder="Digite um comando..."
          autocomplete="off">

        <button
          type="button"
          class="btn btn-primary"
          id="serialSendBtn">
          Enviar
        </button>

      </div>


      <div class="serial-summary">

        <div>
          <span>Baud Rate</span>
          <strong>115200</strong>
        </div>

        <div>
          <span>Placas</span>
          <strong>
            ${state.devices.length}
          </strong>
        </div>

        <div>
          <span>RX</span>
          <strong id="serialRxCount">
            0
          </strong>
        </div>

        <div>
          <span>TX</span>
          <strong id="serialTxCount">
            0
          </strong>
        </div>

      </div>

    </section>
  `;

  const filterButtons =
  document.querySelectorAll(
    "[data-serial-filter]"
  );

filterButtons.forEach(
  button => {
    button.addEventListener(
      "click",
      async () => {
        serialMonitorFilter =
          button.dataset.serialFilter ||
          "ALL";

        filterButtons.forEach(
          item => {
            item.classList.toggle(
              "active",
              item.dataset.serialFilter ===
                serialMonitorFilter
            );
          }
        );

        await loadSerialMonitor();
      }
    );
  }
);

  const clearButton =
    qs("#serialClearBtn");

  clearButton
    ?.addEventListener(
      "click",
      () => {
        const terminal =
          qs("#serialTerminalContent");

        if (terminal) {
          terminal.innerHTML = "";
        }
      }
    );
   const sendButton =
  qs("#serialSendBtn");

const commandInput =
  qs("#serialCommandInput");

sendButton
  ?.addEventListener(
    "click",
    async () => {

      const deviceSelect =
        qs("#serialDeviceSelect");

      const deviceId =
        deviceSelect?.value || "";

      const command =
        commandInput?.value.trim() || "";

      if (!deviceId) {
        toast(
          "Monitor Serial",
          "Selecione um dispositivo."
        );

        return;
      }

      if (!command) {
        toast(
          "Monitor Serial",
          "Digite um comando."
        );

        return;
      }

      try {
        sendButton.disabled = true;

        const response =
          await fetch(
            "/api/serial/send",
            {
              method: "POST",

              headers: {
                "Content-Type":
                  "application/json"
              },

              body: JSON.stringify({
                device_id: deviceId,
                command: command
              })
            }
          );

        const result =
          await response.json();

        if (
          !response.ok ||
          !result.success
        ) {
          throw new Error(
            result.error ||
            "Falha ao enviar comando"
          );
        }

        commandInput.value = "";

        // Atualiza imediatamente o terminal
        // para mostrar o TX.
        await loadSerialMonitor();
      }
      catch (error) {
        console.error(
          "[RIW] Erro TX:",
          error
        );

        toast(
          "Monitor Serial",
          "Erro ao enviar comando."
        );
      }
      finally {
        sendButton.disabled = false;

        commandInput?.focus();
      }
    }
  ); 
}

function renderDevicesSelector(devices) {
  const panel =
    qs("#deviceCapabilitiesPanel");

  if (!panel) {
    return;
  }

  const list =
    Array.isArray(devices)
      ? devices
      : [];

  panel.style.display = "grid";
  panel.innerHTML = "";

  if (list.length === 0) {
    panel.innerHTML = `
      <div class="dashboard-empty">
        Nenhum dispositivo conectado
      </div>
    `;

    return;
  }

  list.forEach(device => {
    const card =
      document.createElement(
        "article"
      );

    card.className =
      "device-info-card";

    const memory =
      device.memory || {};

    // --------------------------------------------------
    // Particularidade de cada hardware
    // --------------------------------------------------

    let description =
      "Dispositivo conectado ao ecossistema TSX-HW.";

    if (
      device.id === "TSX-UNO-001"
    ) {
      description =
        "Placa compacta e versátil, ideal para controle, " +
        "prototipagem, sensores e automação de pequeno porte.";
    }
    else if (
      device.id === "TSX-MEGA-001"
    ) {
      description =
        "Placa com grande quantidade de entradas e saídas, " +
        "indicada para automação, displays e projetos que " +
        "exigem maior número de conexões.";
    }
    else if (
      device.id === "TSX-ESP32-001"
    ) {
      description =
        "Plataforma de alto desempenho para aplicações IoT, " +
        "com conectividade sem fio, maior capacidade de " +
        "processamento e memória.";
    }

    // --------------------------------------------------
    // Memórias disponíveis
    // --------------------------------------------------

    let memoryHtml = "";

    if (memory.flash) {
      memoryHtml += `
        <div class="device-tech-item">
          <span>Flash</span>
          <strong>${memory.flash}</strong>
        </div>
      `;
    }

    if (memory.sram) {
      memoryHtml += `
        <div class="device-tech-item">
          <span>SRAM</span>
          <strong>${memory.sram}</strong>
        </div>
      `;
    }

    if (memory.eeprom) {
      memoryHtml += `
        <div class="device-tech-item">
          <span>EEPROM</span>
          <strong>${memory.eeprom}</strong>
        </div>
      `;
    }

    if (memory.psram) {
      memoryHtml += `
        <div class="device-tech-item">
          <span>PSRAM</span>
          <strong>${memory.psram}</strong>
        </div>
      `;
    }

    // --------------------------------------------------
    // Card
    // --------------------------------------------------

     card.innerHTML = `
  <div class="device-tech-layout">

    <!-- COLUNA ESQUERDA -->
    <div class="device-tech-content">

      <div class="device-tech-header">

        <div class="device-info-status-row">
          <span class="device-info-online">
            Online
          </span>

          <span class="device-info-port">
            ${device.port}
          </span>
        </div>

        <h3 class="device-tech-name">
          ${device.name}
        </h3>

        <div class="device-tech-id">
          ${device.id}
        </div>

      </div>


      <div class="device-tech-grid">

        <!-- HARDWARE -->
        <div class="device-tech-box">

          <div class="device-tech-section-title">
            DADOS DO HARDWARE
          </div>

          <div class="device-tech-item">
            <span>MCU</span>
            <strong>${device.mcu || "-"}</strong>
          </div>

          <div class="device-tech-item">
            <span>Clock</span>
            <strong>${device.clock || "-"}</strong>
          </div>

          <div class="device-tech-item">
            <span>Firmware</span>
            <strong>${device.firmware || "-"}</strong>
          </div>

          <div class="device-tech-item">
            <span>Protocolo</span>
            <strong>${device.protocol || "-"}</strong>
          </div>

        </div>


        <!-- MEMÓRIA -->
        <div class="device-tech-box">

          <div class="device-tech-section-title">
            MEMÓRIA
          </div>

          <div class="device-tech-memory">
            ${memoryHtml}
          </div>

        </div>

      </div>


      <div class="device-tech-description-box">

        <div class="device-tech-section-title">
          PARTICULARIDADE
        </div>

        <div class="device-tech-description">
          ${description}
        </div>

      </div>

    </div>


    <!-- COLUNA DIREITA -->
    <div class="device-tech-visual">

      <div class="device-tech-image">
        <img
          src="${getDeviceImage(device.id)}"
          alt="${device.name}">
      </div>

    </div>

  </div>
 `;
    

    panel.appendChild(card);
  });
}

function renderShieldsDeviceSelector(devices) {
  const panel =
    qs("#deviceCapabilitiesPanel");

  const title =
    qs("#connectedShieldsTitle");

  if (!panel) {
    return;
  }

  if (title) {
    title.style.display = "";
    title.textContent =
      "DISPOSITIVOS E SHIELDS CONECTADOS";
  }

  panel.style.display = "grid";
  // Estamos voltando para o seletor.
  // Libera uma futura renderização do Mega.
  delete panel.dataset.device;
  panel.innerHTML = "";

  devices.forEach(device => {
    const card =
      document.createElement("article");

    card.className =
      "shield-device-card";

    card.innerHTML = `
  <div class="shield-device-card-content">

    <div class="shield-device-left">

      <h3>${device.name}</h3>

      <div class="shield-device-id">
        ${device.id}
      </div>

      <div class="shield-device-status-row">
        <span class="shield-device-online">
          Online
        </span>

        <span class="shield-device-port">
          ${device.port}
        </span>
      </div>

      <button
        type="button"
        class="btn btn-primary device-resources-btn">
        Ver Recursos
      </button>

    </div>

    <div class="shield-device-right">
      <img
        src="${getDeviceImage(device.id)}"
        alt="${device.name}">
    </div>

  </div>
      
    `;

    card
      .querySelector(".device-resources-btn")
      ?.addEventListener(
        "click",
        () => {
          selectDeviceForShields(device);
        }
      );

    panel.appendChild(card);
  });
}

async function loadEsp32Temperature() {
  /*
   * O elemento só existe quando os recursos
   * do ESP32 estão abertos.
   */
  const temperatureElement =
    document.querySelector(
      "#esp32-temperature"
    );

  if (!temperatureElement) {
    return;
  }

  try {
    const response =
      await fetch(
        "/api/serial/monitor",
        {
          method: "GET",
          cache: "no-store"
        }
      );

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }

    const data =
      await response.json();

    const entries =
      Array.isArray(
        data?.entries
      )
        ? data.entries
        : [];

    /*
     * Procura de trás para frente.
     *
     * Usa a leitura válida mais recente
     * do ESP32.
     */
    for (
      let index =
        entries.length - 1;

      index >= 0;

      index--
    ) {
      const entry =
        entries[index];

      if (
        entry?.direction !== "RX" ||
        entry?.device_id !==
          "TSX-ESP32-001" ||
        typeof entry?.text !==
          "string" ||
        !entry.text.includes(
          '"type":"status"'
        )
      ) {
        continue;
      }

      try {
        const status =
          JSON.parse(
            entry.text
          );

        if (
          status?.type !==
            "status" ||
          status?.device_id !==
            "TSX-ESP32-001" ||
          !Array.isArray(
            status?.capabilities
          )
        ) {
          continue;
        }

        const temperature =
          status.capabilities.find(
            capability =>
              capability.id ===
              "temperature"
          );

        /*
         * Importante:
         * não usamos apenas Number(value),
         * porque Number(null) resulta em zero.
         */
        if (
          temperature?.value ===
            null ||
          temperature?.value ===
            undefined
        ) {
          continue;
        }

        const value =
          Number(
            temperature.value
          );

        if (
          !Number.isFinite(
            value
          )
        ) {
          continue;
        }

        temperatureElement.textContent =
          `${value.toFixed(1)} °C`;

        return;
      }
      catch {
        /*
         * A entrada RX pode ser boot,
         * relay, texto comum etc.
         */
      }
    }

    /*
     * Nenhuma leitura válida encontrada.
     * Mantém o valor atual da tela.
     *
     * Não troca para zero nem para --.
     */
  }
  catch (error) {
    console.warn(
      "[RIW] Erro ao atualizar temperatura do ESP32:",
      error
    );
  }
}

let esp32PresenceVisualUntil =
  0;

const ESP32_PRESENCE_VISUAL_HOLD =
  3000;

async function loadEsp32Presence() {
  const presenceElement =
    document.querySelector(
      "#esp32-presence"
    );

  const radarElement =
    document.querySelector(
      "#esp32PresenceRadar"
    );

  if (
    !presenceElement &&
    !radarElement
  ) {
    return;
  }

  try {
    const response =
      await fetch(
        "/api/serial/monitor",
        {
          method: "GET",
          cache: "no-store"
        }
      );

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }

    const data =
      await response.json();

    const entries =
      Array.isArray(data?.entries)
        ? data.entries
        : [];

    let physicalPresence =
      false;

    let statusFound =
      false;

    /*
     * Procura o status mais recente
     * recebido do ESP32.
     */
    for (
      let index =
        entries.length - 1;

      index >= 0;

      index--
    ) {
      const entry =
        entries[index];

      if (
        entry?.direction !== "RX" ||
        entry?.device_id !==
          "TSX-ESP32-001" ||
        typeof entry?.text !==
          "string" ||
        !entry.text.includes(
          '"type":"status"'
        )
      ) {
        continue;
      }

      try {
        const status =
          JSON.parse(
            entry.text
          );

        if (
          status?.type !== "status" ||
          status?.device_id !==
            "TSX-ESP32-001" ||
          !Array.isArray(
            status?.capabilities
          )
        ) {
          continue;
        }

        const presence =
          status.capabilities.find(
            capability =>
              capability.id ===
              "presence"
          );

        if (
          typeof presence?.state !==
            "boolean"
        ) {
          continue;
        }

        physicalPresence =
          presence.state === true;

        statusFound =
          true;

        break;
      }
      catch {
        // Ignora linhas que não sejam JSON válido.
      }
    }

    if (!statusFound) {
      return;
    }

    const now =
      Date.now();

    /*
     * Ao receber TRUE, segura visualmente
     * a detecção por pelo menos três segundos.
     */
    if (physicalPresence) {
      esp32PresenceVisualUntil =
        now +
        ESP32_PRESENCE_VISUAL_HOLD;
    }

    const visualPresence =
      physicalPresence ||
      now <
        esp32PresenceVisualUntil;

    updateEsp32PresenceVisual(
      visualPresence
    );
  }
  catch (error) {
    console.warn(
      "[RIW] Erro ao atualizar presença:",
      error
    );
  }
}

function updateEsp32PresenceVisual(
  detected
) {
  const presenceElement =
    document.querySelector(
      "#esp32-presence"
    );

  const radarElement =
    document.querySelector(
      "#esp32PresenceRadar"
    );

  const presenceCard =
    presenceElement?.closest(
      ".shield-card"
    ) ||
    presenceElement?.parentElement
      ?.parentElement;

  if (presenceElement) {
    presenceElement.textContent =
      detected
    ? "Detectada"
    : "Não detectada";

    presenceElement.classList.toggle(
      "detected",
      detected
    );

    presenceElement.classList.toggle(
      "not-detected",
      !detected
    );
  }

  if (radarElement) {
    radarElement.classList.toggle(
      "detected",
      detected
    );
  }

  if (presenceCard) {
    presenceCard.classList.toggle(
      "presence-active",
      detected
    );
  }
}

async function loadSerialMonitor() {
  // Só atualiza quando estamos realmente
  // dentro do Monitor Serial.
  if (state.currentView !== "serial") {
    return;
  }

  try {
    const response =
      await fetch(
        "/api/serial/monitor",
        {
          method: "GET",
          cache: "no-store"
        }
      );

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }

    const data =
      await response.json();

    const terminal =
      qs("#serialTerminalContent");

    const rxCount =
      qs("#serialRxCount");
     
    const txCount =
      qs("#serialTxCount");

     if (!terminal) {
      return;
     }

    const entries =
      Array.isArray(data?.entries)
        ? data.entries
        : [];

    const visibleEntries =
       serialMonitorFilter === "ALL"
       ? entries
       : entries.filter(
        entry =>
          entry.device_id ===
          serialMonitorFilter
      );    

      // Conta RX real
     const totalRx =
      visibleEntries.filter(
        entry =>
          entry.direction === "RX"
      ).length;

     const totalTx =
      visibleEntries.filter(
      entry =>
      entry.direction === "TX"
      ).length; 

      if (rxCount) {
      rxCount.textContent =
      String(totalRx);
      }

      if (txCount) {
      txCount.textContent =
      String(totalTx);
      }

           terminal.innerHTML =
            visibleEntries
        .map(entry => {

          const device =
            entry.device_id ||
            entry.port ||
            "-";

          return `
            <div class="serial-line">

              <span class="serial-time">
                ${entry.time}
              </span>

              <span class="serial-direction">
                ${entry.direction}
              </span>

              <span class="serial-text">
                [${entry.port}]
                [${device}]
                ${escapeSerialHtml(entry.text)}
              </span>

            </div>
          `;
        })
        .join("");

    // Mantém o terminal sempre na última linha
    terminal.scrollTop =
      terminal.scrollHeight;
  }
  catch (error) {
    console.warn(
      "[RIW] Erro Monitor Serial:",
      error
    );
  }
}

function escapeSerialHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}


function selectDeviceForShields(device) {
   state.shieldsMode = "resources";
   state.selectedShieldDeviceId = device.id;

    const deviceData = {
    id: device.id,
    name: device.name,
    port: device.port,
    firmware: "-",
    protocol: "TSX-HW/1.0"
  };

  switch (device.id) {

    case "TSX-UNO-001":
      showUnoDevice(deviceData);
      break;

    case "TSX-MEGA-001":
      showMegaDevice(deviceData);
      break;

    case "TSX-ESP32-001":
      showEsp32Device(deviceData);
      renderEsp32Capabilities();
      break;
  }
}

function getDefaultGalleryDate() {
  const now =
    new Date();

  const year =
    now.getFullYear();

  const month =
    String(
      now.getMonth() + 1,
    ).padStart(
      2,
      "0",
    );

  const day =
    String(
      now.getDate(),
    ).padStart(
      2,
      "0",
    );

  const today =
    `${year}-${month}-${day}`;

  if (
    galleryEventDates.includes(
      today,
    )
  ) {
    return today;
  }

  return "2026-08-04";
}

function updateGalleryDaySelection() {
  qsa(
    "[data-gallery-date]"
  ).forEach(
    button => {
      const date =
        button.dataset.galleryDate;

      button.classList.toggle(
        "active",
        date === gallerySelectedDate
      );
    }
  );
}

function bindGalleryDays() {
  qsa(
    "[data-gallery-date]"
  ).forEach(
    button => {
      button.addEventListener(
        "click",
        async () => {
          const selectedDate =
            button.dataset.galleryDate;

          gallerySelectedDate =
            selectedDate;

          gallerySignatureIndex =
           0;

           stopGallerySignatureCarousel();  

          galleryLastSignatureCount =
            -1;

            updateGalleryDaySelection();

          await loadGallerySignatures(
            selectedDate
          );

          await loadGalleryPhotos(
            selectedDate
          );
        }
      );
    }
  );
}


async function loadGallerySignatures(
  eventDate = null
) {
  const carousel =
    qs("#gallerySignaturesCarousel");

  const count =
    qs("#gallerySignaturesCount");

  if (!carousel) {
    return;
  }

  if (!eventDate) {
    eventDate =
      gallerySelectedDate ||
      getDefaultGalleryDate();
  }

  gallerySelectedDate =
    eventDate;

  updateGalleryDaySelection();

  try {
    const response =
      await fetch(
        `/api/gallery/signatures?date=${encodeURIComponent(
          eventDate
        )}`,
        {
          method: "GET",
          cache: "no-store"
        }
      );

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }

    const data =
      await response.json();

    const signatures =
      Array.isArray(
        data.signatures
      )
        ? data.signatures
        : [];

    const previousCount =
      galleryLastSignatureCount;

    galleryLastSignatureCount =
      signatures.length;

    if (count) {
      count.textContent =
        `${signatures.length} ${
          signatures.length === 1
            ? "registro"
            : "registros"
        }`;
    }

    if (
      signatures.length === 0
    ) {
      stopGallerySignatureCarousel();

      gallerySignatureIndex =
        0;

      carousel.innerHTML = `
        <div class="gallery-empty">
          <span>✍</span>

          <strong>
            Nenhuma assinatura registrada
          </strong>

          <p>
            As assinaturas deste dia
            aparecerão aqui.
          </p>
        </div>
      `;

      return;
    }

    const hasNewEntry =
      previousCount >= 0 &&
      signatures.length >
        previousCount;

    if (hasNewEntry) {
      gallerySignatureIndex =
        signatures.length - 1;
    }
    else if (
      gallerySignatureIndex >=
      signatures.length
    ) {
      gallerySignatureIndex =
        0;
    }

    carousel.innerHTML = `
      <div
        class="gallery-carousel-track"
        id="gallerySignatureTrack"
      >
        ${signatures
          .map(
            (
              fileName,
              index
            ) => `
              <div
                class="
                  gallery-signature-item
                  ${
                    hasNewEntry &&
                    index ===
                      signatures.length - 1
                      ? "new-entry"
                      : ""
                  }
                "
              >
                <img
                  src="/api/gallery/signatures/file?date=${encodeURIComponent(
                    eventDate
                  )}&name=${encodeURIComponent(
                    fileName
                  )}"
                  alt="Assinatura do evento"
                >
              </div>
            `
          )
          .join("")}
      </div>

      ${
        signatures.length > 1
          ? `
            <button
              type="button"
              class="
                gallery-carousel-button
                gallery-carousel-prev
              "
              id="gallerySignaturePrev"
            >
              ‹
            </button>

            <button
              type="button"
              class="
                gallery-carousel-button
                gallery-carousel-next
              "
              id="gallerySignatureNext"
            >
              ›
            </button>
          `
          : ""
      }

      <div
        class="gallery-carousel-position"
        id="gallerySignaturePosition"
      >
      </div>
    `;

    bindGallerySignatureCarousel(
      signatures.length
    );

    updateGallerySignatureCarousel(
      signatures.length
    );

    startGallerySignatureCarousel(
      signatures.length
    );
  }
  catch (error) {
    console.error(
      "[RIW] Falha ao carregar mural:",
      error
    );

    carousel.innerHTML = `
      <div class="gallery-empty">
        <strong>
          Não foi possível carregar
          as assinaturas.
        </strong>
      </div>
    `;
  }
}  

async function loadGalleryPhotos(
  eventDate = null
) {
  const carousel =
    qs("#galleryPhotosCarousel");

  const count =
    qs("#galleryPhotosCount");

  if (!carousel) {
    return;
  }

  if (!eventDate) {
    eventDate =
      gallerySelectedDate ||
      getDefaultGalleryDate();
  }

  try {
    const response =
      await fetch(
        `/api/gallery/photos?date=${encodeURIComponent(
          eventDate
        )}`,
        {
          method: "GET",
          cache: "no-store"
        }
      );

    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }

    const data =
      await response.json();

    const photos =
      Array.isArray(
        data.photos
      )
        ? data.photos
        : [];

    if (count) {
      count.textContent =
        `${photos.length} ${
          photos.length === 1
            ? "registro"
            : "registros"
        }`;
    }

    if (photos.length === 0) {
      stopGalleryPhotoCarousel();

        galleryPhotoIndex =
        0;

        galleryLastPhotoCount =
        0;


      carousel.innerHTML = `
        <div class="gallery-photo-empty">

          <span>📷</span>

          <strong>
            Aguardando os primeiros momentos...
          </strong>

          <small>
            As fotos deste dia aparecerão automaticamente.
          </small>

        </div>
      `;

      return;
    }

    /*
     * Por enquanto mostramos a foto
     * mais recente.
     */
    const previousCount =
  galleryLastPhotoCount;

galleryLastPhotoCount =
  photos.length;

if (
  previousCount >= 0 &&
  photos.length >
    previousCount
) {
  galleryPhotoIndex =
    photos.length - 1;
}
else if (
  galleryPhotoIndex >=
  photos.length
) {
  galleryPhotoIndex =
    0;
}

updateGalleryPhotoCarousel(
  photos,
  eventDate
);

startGalleryPhotoCarousel(
  photos,
  eventDate
);  

    console.log(
      "[RIW] Fotos carregadas:",
      photos.length
    );
  }
  catch (error) {
    console.error(
      "[RIW] Falha ao carregar fotos do mural:",
      error
    );

    carousel.innerHTML = `
      <div class="gallery-photo-empty">

        <strong>
          Não foi possível carregar as fotos.
        </strong>

      </div>
    `;
  }
}


function getGallerySignatureVisibleCount() {
  const container =
    qs("#gallerySignaturesCarousel");

  if (!container) {
    return 1;
  }

  const availableWidth =
    container.clientWidth;

  const itemWidth =
    180;

  const gap =
    12;

  return Math.max(
    1,
    Math.floor(
      availableWidth /
      (itemWidth + gap)
    )
  );
}

function updateGallerySignatureCarousel(
  total
) {
  if (total <= 0) {
    return;
  }

  const track =
    qs("#gallerySignatureTrack");

  if (!track) {
    return;
  }

  const visibleCount =
    getGallerySignatureVisibleCount();

  const maxIndex =
    Math.max(
      0,
      total - visibleCount
    );

  if (
    gallerySignatureIndex >
    maxIndex
  ) {
    gallerySignatureIndex =
      maxIndex;
  }

  const itemWidth =
    180;

  const gap =
    12;

  const offset =
    gallerySignatureIndex *
    (itemWidth + gap);

  track.style.transform =
    `translateX(-${offset}px)`;
}

function bindGallerySignatureCarousel(
  total
) {
  const previous =
    qs("#gallerySignaturePrev");

  const next =
    qs("#gallerySignatureNext");

  previous?.addEventListener(
    "click",
    () => {
      gallerySignatureIndex =
        (
          gallerySignatureIndex -
          1 +
          total
        ) %
        total;

      updateGallerySignatureCarousel(
        total
      );

      startGallerySignatureCarousel(
        total
      );
    }
  );

  next?.addEventListener(
    "click",
    () => {
      gallerySignatureIndex =
        (
          gallerySignatureIndex +
          1
        ) %
        total;

      updateGallerySignatureCarousel(
        total
      );

      startGallerySignatureCarousel(
        total
      );
    }
  );
}

function stopGallerySignatureCarousel() {
  if (gallerySignatureTimer) {
    clearInterval(
      gallerySignatureTimer
    );

    gallerySignatureTimer =
      null;
  }
}

function startGallerySignatureCarousel(
  total
) {
  stopGallerySignatureCarousel();

  const visibleCount =
    getGallerySignatureVisibleCount();

  if (
    total <= visibleCount
  ) {
    gallerySignatureIndex =
      0;

    updateGallerySignatureCarousel(
      total
    );

    return;
  }

  gallerySignatureTimer =
    setInterval(
      () => {
        if (
          state.currentView !==
          "gallery"
        ) {
          return;
        }

        const currentVisibleCount =
          getGallerySignatureVisibleCount();

        const maxIndex =
          Math.max(
            0,
            total -
            currentVisibleCount
          );

        if (
          gallerySignatureIndex >=
          maxIndex
        ) {
          gallerySignatureIndex =
            0;
        }
        else {
          gallerySignatureIndex++;
        }

        updateGallerySignatureCarousel(
          total
        );
      },
      4000
    );
}


function stopGalleryAutoRefresh() {
  if (galleryRefreshTimer) {
    clearInterval(
      galleryRefreshTimer
    );

    galleryRefreshTimer =
      null;
  }
}

function startGalleryAutoRefresh() {
  stopGalleryAutoRefresh();

  galleryRefreshTimer =
    setInterval(
      async () => {
        if (
          state.currentView !==
            "gallery" ||
          !gallerySelectedDate
        ) {
          return;
        }

        // ==================================================
        // ASSINATURAS
        // ==================================================

        try {
          const response =
            await fetch(
              `/api/gallery/signatures?date=${encodeURIComponent(
                gallerySelectedDate,
              )}`,
              {
                cache: "no-store",
              },
            );

          if (response.ok) {
            const data =
              await response.json();

            const count =
              Number(
                data.count ?? 0
              );

            if (
              galleryLastSignatureCount ===
              -1
            ) {
              galleryLastSignatureCount =
                count;
            }
            else if (
              count !==
              galleryLastSignatureCount
            ) {
              galleryLastSignatureCount =
                count;

              await loadGallerySignatures(
                gallerySelectedDate
              );
            }
          }
        }
        catch (error) {
          console.error(
            "[RIW] Falha ao atualizar assinaturas do mural:",
            error
          );
        }


        // ==================================================
        // FOTOS
        // ==================================================

        try {
          const photoResponse =
            await fetch(
              `/api/gallery/photos?date=${encodeURIComponent(
                gallerySelectedDate
              )}`,
              {
                cache: "no-store"
              }
            );

          if (photoResponse.ok) {
            const photoData =
              await photoResponse.json();

            const photoCount =
              Number(
                photoData.count ?? 0
              );

            if (
              galleryLastPhotoCount ===
              -1
            ) {
              galleryLastPhotoCount =
                photoCount;
            }
            else if (
              photoCount !==
              galleryLastPhotoCount
            ) {
              galleryLastPhotoCount =
                photoCount;

              await loadGalleryPhotos(
                gallerySelectedDate
              );
            }
          }
        }
        catch (error) {
          console.error(
            "[RIW] Falha ao atualizar fotos do mural:",
            error
          );
        }
      },
      1500
    );
}


function leaveGalleryView() {
  document.body.classList.remove(
    "gallery-mode"
  );

  const layout =
  qs(".layout");

  const sidebar =
  qs(".sidebar");

  const mainHeader =
  qs(".topbar");

  if (sidebar) {
  sidebar.style.display =
    "";
  }

  if (mainHeader) {
  mainHeader.style.display =
    "";
  }

  if (layout) {
  layout.style.display =
    "";
  }

  const gallery =
    qs("#gallerySection");

  if (gallery) {
    gallery.style.display =
      "none";
  }

  stopGalleryAutoRefresh();
  stopGallerySignatureCarousel();

  showDashboardView();

  qsa(".nav-item").forEach(
    item => {
      item.classList.toggle(
        "active",
        item.textContent.includes(
          "Dashboard"
        )
      );
    }
  );
}