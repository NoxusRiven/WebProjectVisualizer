const path = "/TODO";

let editingElement = null;
let addingElement = null;

// Pobierz wszystkie linki z menu
const projLinks = document.querySelectorAll('.project_list a');

// Pobierz aktualną ścieżkę (np. "/oferta.html")
const currentPath = window.location;

document.addEventListener("change", onChange);
document.addEventListener("dblclick", onDoubleClick);
document.addEventListener("click", onClick);

document.addEventListener("mousedown", (e) =>
{
    if (!editingElement)
        return;

    if (editingElement.contains(e.target))
        return;

    finishEditing(editingElement);
});

document.addEventListener("mousedown", (e) =>
{
    if (!addingElement)
        return;

    if (addingElement.contains(e.target))
        return;

    addingElement.submit();
});

projLinks.forEach(link => 
{
  // Sprawdzamy, czy href linku pasuje do obecnej ścieżki
  // Używamy .pathname z obiektu URL, aby obsłużyć też pełne adresy
    const linkUrl = new URL(link.href, window.location.origin);
    
    if (linkUrl.href === currentPath.href)
    {
        link.classList.add('highlight');
    }
});

function onDoubleClick(event)
{
    if (!event.target.matches("p"))
        return;

    const parent = event.target.closest("[data-id]");

    if (!parent)
        return;

    startEditing(parent);
}

function startEditing(parent)
{
    if (editingElement)
        finishEditing(editingElement);

    if (parent.querySelector("input:not([type='checkbox'])"))
        return;

    editingElement = parent;

    const p = parent.querySelector("p");

    const input = document.createElement("input");
    input.value = p.textContent;

    for (const child of parent.children)
    {
        if (
            child.matches("p") ||
            (child.matches("input") && child.type === "checkbox")
        )
        {
            child.style.display = "none";
        }
    }

    parent.appendChild(input);

    input.focus();
    input.select();

    input.addEventListener("keydown", (e) =>
    {
        if (e.key === "Enter")
            finishEditing(parent);

        if (e.key === "Escape")
        {
            input.remove();

            for (const child of parent.children)
            {
                child.style.display = "";
            }

            editingElement = null;
        }
    });
}

function finishEditing(parent)
{
    if (!parent)
        return;

    const input = parent.querySelector("input:not([type='checkbox'])");

    if (input)
    {
        const p = parent.querySelector("p");

        p.textContent = input.value.trim();

        input.remove();

        for (const child of parent.children)
        {
            child.style.display = "";
        }
    }

    editingElement = null;

    put(parent);
}

function onChange(event)
{
    if (event.target.type !== "checkbox")
        return;

    const parent = event.target.closest("[data-id]");

    if (!parent)
        return;

    put(parent);
}

function onClick(event)
{
    const priority = event.target.closest(".priority");

    if (!priority)
        return;

    const parent = priority.closest("[data-id]");

    if (!parent)
        return;

    if (editingElement !== parent)
        return;

    if (priority.classList.contains("low"))
    {
        priority.classList.remove("low");
        priority.classList.add("medium");
    }
    else if (priority.classList.contains("medium"))
    {
        priority.classList.remove("medium");
        priority.classList.add("high");
    }
    else
    {
        priority.classList.remove("high");
        priority.classList.add("low");
    }
}

//TODO: web side adding new form and then when data was inputed by user sending it 
// to server for saving and adding id
//callback from html
function addElement(btn)
{
    console.log("editing ele: ", editingElement)
    console.log("adding ele: ", addingElement)
    
    if (editingElement)
        finishEditing(editingElement);

    if (addingElement)
        addingElement.submit();

    const request =
    {
        type: btn.dataset.type
    };

    console.log("btn type: ", btn.dataset.type)

    if (btn.dataset.parent)
        request.parentID = Number(btn.dataset.parent);

    let base;
    let input;
    let priority = null;

    if (request.type === "objective")
    {
        base = document.createElement("div");
        base.className = "objective";

        priority = document.createElement("div");
        priority.className = "priority low";

        input = document.createElement("input");

        base.appendChild(priority);
        base.appendChild(input);

        priority.addEventListener("click", () =>
        {
            if (priority.classList.contains("low"))
                priority.classList.replace("low", "medium");
            else if (priority.classList.contains("medium"))
                priority.classList.replace("medium", "high");
            else
                priority.classList.replace("high", "low");
        });
    }
    else
    {
        input = document.createElement("input");
        base = input;

        input.className =
            request.type === "task"
                ? "task-indent"
                : "comment-indent";
    }

    btn.parentNode.insertBefore(base, btn);

    input.focus();
    input.select();


    
    function submit()
    {
        const value = input.value;

        if (value === "")
        {
            base.remove();
            return;
        }

        request.value = value.trim();

        if (priority)
        {
            if (priority.classList.contains("low"))
                request.priority = 0;
            else if (priority.classList.contains("medium"))
                request.priority = 1;
            else
                request.priority = 2;
        }

        fetch("/TODO", {
            method: "POST",
            headers:
            {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(request)
        })
        .then(response =>
        {
            if (!response.ok)
                throw new Error("POST failed");

            base.remove();
            addingElement = null;

            location.reload();
        })
        .catch(console.error);
    }

    base.submit = submit 
    addingElement = base;

    input.addEventListener("keydown", (e) =>
    {
        if (e.key === "Escape")
        {
            base.remove();
            addingElement = null;
            return;
        }

        if (e.key === "Enter")
        {
            submit();
        }
    });

}

function put(parent)
{
    fetch(path,
    {
        method: "PUT",
        headers:
        {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(createPut(parent))
    });
}

function createPut(parent)
{
    const put =
    {
        id: Number(parent.dataset.id)
    };

    const text = parent.querySelector("p");

    if (text)
        put.value = text.textContent.trim();

    const checkbox = parent.querySelector("input[type='checkbox']");

    if (checkbox)
        put.finished = checkbox.checked;

    const priority = parent.querySelector(".priority");

    if (priority)
    {
        if (priority.classList.contains("low"))
            put.priority = 0;
        else if (priority.classList.contains("medium"))
            put.priority = 1;
        else if (priority.classList.contains("high"))
            put.priority = 2;
    }

    return put;
}
