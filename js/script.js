// Set copyright year
document.getElementById("year").insertAdjacentHTML("afterbegin", new Date().getFullYear());

// OS ditection
let os;
const ua = window.navigator.userAgent.toLowerCase();
const winMatch = ua.match(/windows nt (\d+\.\d)/i);
if (winMatch) {
  const version = winMatch[1];
  if (version >= 10.0) os = "windows-10";
  else if (version >= 6.0) os = "windows-7";
  else if (version >= 5.1) os = "windows-xp";
  else os = "other";
}
else if (ua.indexOf("linux") !== -1) {
  os = "linux";
}
else if (ua.indexOf("mac os x") !== -1) {
  os = "macos";
}
else {
  os = "other";
}

// Fetch data from Github API
fetch("https://api.github.com/repos/BambooTracker/BambooTracker/releases/latest", {
  method: "GET"
})
.then(resp => resp.ok ? resp.json() : new Promise.reject())
.then(data => {
  // Update dropdown header
  const head = document.getElementById("dl-head");
  const releaseName = data["name"];
  const releaseDate = data["published_at"].match(/^([^T]+)/)[1];
  head.insertAdjacentHTML("beforeend", " " + releaseName + " (" + releaseDate + ")");
  document.getElementById("release-note").href = data["html_url"];
  if (os == "other") {
    head.href = data["html_url"];
  }

  // Set download links
  for (const asset of data["assets"]) {
    const name = asset["name"];
    const dlUrl = asset["browser_download_url"];

    if (name.endsWith("windows-10-64bit.zip")) {
      const item = document.getElementById("dl-windows-10");
      item.href = dlUrl;
      item.classList.remove("disabled");
      if (os == "windows-10") {
        head.href = dlUrl;
      }
    }
    else if (name.endsWith("windows-7-32bit.zip")) {
      const item = document.getElementById("dl-windows-7");
      item.href = dlUrl;
      item.classList.remove("disabled");
      if (os == "windows-7") {
        head.href = dlUrl;
      }
    }
    else if (name.endsWith("windows-xp-32bit.zip")) {
      const item = document.getElementById("dl-windows-xp");
      item.href = dlUrl;
      item.classList.remove("disabled");
      if (os == "windows-xp") {
        head.href = dlUrl;
      }
    }
    else if (name.endsWith("macos-64bit.zip")) {
      const item = document.getElementById("dl-macos");
      item.href = dlUrl;
      item.classList.remove("disabled");
      if (os == "macos") {
        head.href = dlUrl;
      }
    }
    else if (name.endsWith("linux-64bit.zip")) {
      const item = document.getElementById("dl-linux");
      item.href = dlUrl;
      item.classList.remove("disabled");
      if (os == "linux") {
        head.href = dlUrl;
      }
    }
  }
});
