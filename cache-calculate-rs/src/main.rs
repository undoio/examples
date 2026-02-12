use rand::Rng;

#[derive(Clone, Copy)]
struct CacheEntry {
    number: u8,
    sqroot: u8,
}

const CACHE_SIZE: usize = 100;

#[inline(never)]
fn cache_calculate(number: i32, cache: &mut [CacheEntry]) -> u8 {
    if let Some(entry) = cache.iter().find(|entry| entry.number == number as u8) {
        return entry.sqroot;
    }

    let mut rng = rand::rng();

    let mut sqroot = 0;

    for number_adj in (number - 1)..=(number + 1) {
        let sqroot_adj = (number_adj as f64).sqrt();

        let i = rng.random_range(0..CACHE_SIZE);

        cache[i].number = number_adj as u8;
        cache[i].sqroot = sqroot_adj as u8;

        if number_adj == number {
            sqroot = sqroot_adj as u8;
        }
    }

    sqroot
}

fn main() {
    let mut cache = [CacheEntry {
        number: 0,
        sqroot: 0,
    }; CACHE_SIZE];

    let mut rng = rand::rng();

    let mut iter_count = 0;
    loop {
        if iter_count % 100 == 0 {
            println!("i={iter_count}");
        }

        let number: u8 = rng.random();
        let sqroot_cache = cache_calculate(number as i32, &mut cache);
        let sqroot_correct = (number as f64).sqrt() as u8;

        assert!(sqroot_cache == sqroot_correct);

        iter_count += 1;
    }
}
