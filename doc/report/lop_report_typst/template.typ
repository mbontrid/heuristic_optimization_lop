
#let styling(it) = {
  /////////////////////////////////////
  // Page
  /////////////////////////////////////
  set page(
    paper: "a4",
    number-align: center,
    margin: 100pt,
  )

  set text(font: "New Computer Modern")


  /////////////////////////////////////
  // Outline
  /////////////////////////////////////

  show link: set text(fill: blue)
  show outline.entry.where(): it => {
    text(size: 15pt)[#it]
  }
  show outline.entry.where(level: 1): set text(weight: "bold")
  show outline.entry.where(level: 1): set outline.entry(fill: none)
  show outline.entry.where(level: 1): it => {
    v(0.7em)
    it
  }

  show outline.entry: it => {
    let no-nums = query(label("abbr"))
    let no-nums2 = query(label("ack"))
    if it.element in no-nums or it.element in no-nums2 {} else {
      it
    }
  }


  /////////////////////////////////////
  // Heading
  /////////////////////////////////////
  set par(first-line-indent: 1.5em)
  show selector.or(heading, table, grid, figure): it => {
    it
    ""
    context v(-par.spacing - measure("").height)
  }
  set heading(numbering: "1.1 ")

  show heading.where(level: 1): it => {
    if (
      counter(heading).display() != "0"
        and it.body.text != "Bibliography"
        and it.body.text != "Abbreviations"
        and it.body.text != "Acknowledgments"
    ) {
      pagebreak()
      grid(
        columns: 1fr,
        row-gutter: 1.2em,
        [#text(size: 1.3em)[Chapter #counter(heading).display()]],
        text(size: 1.5em)[#it.body],
        []
      )
    } else {
      text(size: 1.5em)[#it.body]
      "\n"
    }
  }
  set heading(numbering: "1.1 ")

  show bibliography: set heading(numbering: none)

  /////////////////////////////////////
  // Equation
  /////////////////////////////////////

  show math.equation: set text(12pt)

  /////////////////////////////////////
  // Equation
  /////////////////////////////////////
  show figure.caption: it => {
    v(0.7em)
    it
  }
  show figure: it => {
    v(0.5em)
    it
    "\n"
  }
  it
}


#let title_page = {
  v(1fr)
  align(center)[#image("sceau-a-quadri.webp", width: 40%)]
  v(1fr)


  align(center + horizon)[
    #line(length: 100%, stroke: 2pt)
    #text(size: 34pt)[Heuristic optimization: LOP]
    #line(length: 100%, stroke: 2pt)
  ]

  v(1fr)
  align(center)[
    #text(size: 18pt)[
      Author: Milan Bontridder \

      \
      Academic year 2025-2026
      #v(1fr)
    ]
  ]
}

#let equation(title, equation, lab: " ") = context {
  import "@preview/showybox:2.0.4": showybox

  // Query for the equation to be placed so that we can get
  // its location and its counter value.
  let eq = query(selector(math.equation).after(here())).at(0, default: none)
  let title = if eq != none {
    let nums = counter(figure.where(kind: "equation")).at(eq.location())
    [#eq.supplement #numbering("1.1", ..nums): #title]
  }


  // Hide the numbering in the equation itself.
  set math.equation(numbering: (..) => none)
  [#figure(
      showybox(
        title-style: (
          boxed-style: (anchor: (x: center, y: horizon), radius: (top-left: 10pt, bottom-right: 10pt, rest: 0pt)),
          color: black,
        ),
        frame: (
          title-color: blue.lighten(80%),
          body-color: blue.lighten(90%),
          border-color: navy.darken(20%),
          radius: (top-left: 10pt, bottom-right: 10pt, rest: 0pt),
        ),
        title: title,
        [#equation], // check equation
      ),
      kind: "equation",
      supplement: [Equation],
    )
    #label(lab)
  ]
}

