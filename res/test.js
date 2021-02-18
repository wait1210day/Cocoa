let a = [1, 2, 3, 4];
let b = [1, 2, 3];

for (let sa in a)
{
    for (let sb in b)
    {
        let delta = sa * sa - 8 * sb;
        console.log(delta);
    }
}

let node = document.getElementById("footbar");
node.node