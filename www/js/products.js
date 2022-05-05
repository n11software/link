let products = document.createElement("div")
products.className = "products"
{
  let featured = document.createElement("div")
  featured.className = "featured"
  let featuredTitle = document.createElement("span")
  featuredTitle.innerHTML = "Featured"
  featuredTitle.className = "title"
  featured.appendChild(featuredTitle)
  let featuredProducts = document.createElement("div")
  featuredProducts.className = "featured-products"
  {
    let lambda = document.createElement("div")
    lambda.onclick = () => Route("/products/lambda")
    lambda.className = "product"
    let lambdaTitle = document.createElement("span")
    lambdaTitle.innerHTML = "Lambda"
    lambdaTitle.className = "title"
    let lambdaDescription = document.createElement("span")
    lambdaDescription.innerHTML = "A game engine."
    lambdaDescription.className = "description"
    lambda.appendChild(lambdaTitle)
    lambda.appendChild(lambdaDescription)
    featuredProducts.appendChild(lambda)
  }
  {
    let kaos = document.createElement("div")
    kaos.onclick = () => Route("/products/kaos")
    kaos.className = "product"
    let kaosTitle = document.createElement("span")
    kaosTitle.innerHTML = "Kaos"
    kaosTitle.className = "title"
    let kaosDescription = document.createElement("span")
    kaosDescription.innerHTML = "A platform for gamers."
    kaosDescription.className = "description"
    kaos.appendChild(kaosTitle)
    kaos.appendChild(kaosDescription)
    featuredProducts.appendChild(kaos)
  }
  {
    let hydra = document.createElement("div")
    hydra.onclick = () => Route("/products/hydra")
    hydra.className = "product"
    let hydraTitle = document.createElement("span")
    hydraTitle.innerHTML = "Hydra"
    hydraTitle.className = "title"
    let hydraDescription = document.createElement("span")
    hydraDescription.innerHTML = "A modern Operating System."
    hydraDescription.className = "description"
    hydra.appendChild(hydraTitle)
    hydra.appendChild(hydraDescription)
    featuredProducts.appendChild(hydra)
  }
  {
    let nuclear = document.createElement("div")
    nuclear.onclick = () => Route("/products/nuclear")
    nuclear.className = "product"
    let nuclearTitle = document.createElement("span")
    nuclearTitle.innerHTML = "Nuclear"
    nuclearTitle.className = "title"
    let nuclearDescription = document.createElement("span")
    nuclearDescription.innerHTML = "A low level language."
    nuclearDescription.className = "description"
    nuclear.appendChild(nuclearTitle)
    nuclear.appendChild(nuclearDescription)
    featuredProducts.appendChild(nuclear)
  }
  {
    let neon = document.createElement("div")
    neon.onclick = () => Route("/products/neon")
    neon.className = "product"
    let neonTitle = document.createElement("span")
    neonTitle.innerHTML = "Neon"
    neonTitle.className = "title"
    let neonDescription = document.createElement("span")
    neonDescription.innerHTML = "A Version Control System."
    neonDescription.className = "description"
    neon.appendChild(neonTitle)
    neon.appendChild(neonDescription)
    featuredProducts.appendChild(neon)
  }
  featured.appendChild(featuredProducts)
  products.appendChild(featured)
}
{
  let other = document.createElement("div")
  other.className = "other"
  let GamingCoding = document.createElement("div")
  GamingCoding.className = "gaming-coding"
  let Gaming = document.createElement("div")
  Gaming.className = "gaming"
  let GamingTitle = document.createElement("span")
  GamingTitle.innerHTML = "Gaming"
  GamingTitle.className = "title"
  Gaming.appendChild(GamingTitle)
  {
    let GamingProducts = document.createElement("div")
    GamingProducts.className = "gaming-products"
    let kaos = document.createElement("div")
    kaos.onclick = () => Route("/products/kaos")
    kaos.className = "product"
    kaos.innerHTML = "Kaos"
    GamingProducts.appendChild(kaos)
    let lambda = document.createElement("div")
    lambda.onclick = () => Route("/products/lambda")
    lambda.className = "product"
    lambda.innerHTML = "Lambda"
    GamingProducts.appendChild(lambda)
    let n11client = document.createElement("div")
    n11client.onclick = () => Route("/products/n11client")
    n11client.className = "product"
    n11client.innerHTML = "N11 Client"
    GamingProducts.appendChild(n11client)
    let nmc = document.createElement("div")
    nmc.onclick = () => Route("/products/nmc")
    nmc.className = "product"
    nmc.innerHTML = "NotMinecraft"
    GamingProducts.appendChild(nmc)
    Gaming.appendChild(GamingProducts)
  }
  GamingCoding.appendChild(Gaming)
  let Coding = document.createElement("div")
  Coding.className = "coding"
  let CodingTitle = document.createElement("span")
  CodingTitle.innerHTML = "Coding"
  CodingTitle.className = "title"
  Coding.appendChild(CodingTitle)
  {
    let CodingProducts = document.createElement("div")
    CodingProducts.className = "coding-products"
    let lambda = document.createElement("div")
    lambda.onclick = () => Route("/products/lambda")
    lambda.className = "product"
    lambda.innerHTML = "Lambda"
    CodingProducts.appendChild(lambda)
    let hydra = document.createElement("div")
    hydra.onclick = () => Route("/products/hydra")
    hydra.className = "product"
    hydra.innerHTML = "Hydra"
    CodingProducts.appendChild(hydra)
    let nuclear = document.createElement("div")
    nuclear.onclick = () => Route("/products/nuclear")
    nuclear.className = "product"
    nuclear.innerHTML = "Nuclear"
    CodingProducts.appendChild(nuclear)
    let neon = document.createElement("div")
    neon.onclick = () => Route("/products/neon")
    neon.className = "product"
    neon.innerHTML = "Neon"
    CodingProducts.appendChild(neon)
    let link = document.createElement("div")
    link.onclick = () => Route("/products/link")
    link.className = "product"
    link.innerHTML = "Link"
    CodingProducts.appendChild(link)
    Coding.appendChild(CodingProducts)
  }
  GamingCoding.appendChild(Coding)
  other.appendChild(GamingCoding)
  let Servers = document.createElement("div")
  Servers.className = "servers"
  let ServersTitle = document.createElement("span")
  ServersTitle.innerHTML = "Servers"
  ServersTitle.className = "title"
  Servers.appendChild(ServersTitle)
  {
    let ServersProducts = document.createElement("div")
    ServersProducts.className = "servers-products"
    let xeon = document.createElement("div")
    xeon.onclick = () => Route("/products/xeon")
    xeon.className = "product"
    xeon.innerHTML = "Xeon"
    ServersProducts.appendChild(xeon)
    let link = document.createElement("div")
    link.onclick = () => Route("/products/link")
    link.className = "product"
    link.innerHTML = "Link"
    ServersProducts.appendChild(link)
    Servers.appendChild(ServersProducts)
  }
  other.appendChild(Servers)
  products.appendChild(other)
}