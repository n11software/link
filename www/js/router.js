let Route = (url) => {
  let HTTP = new XMLHttpRequest()
  HTTP.open('GET', url)
  HTTP.send()
  HTTP.onreadystatechange = error => {
    document.querySelector('html').innerHTML=(HTTP.responseText)
    window.history.pushState(null, '', url);
  }
}