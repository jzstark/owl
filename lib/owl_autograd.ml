(*
 * OWL - an OCaml numerical library for scientific computing
 * Copyright (c) 2016-2017 Liang Wang <liang.wang@cl.cam.ac.uk>
 *)

type scalar = Float of float | Node of node
and node = {
  v : scalar;
  d : scalar;
}

let rec print_node_helper x =
  match x with
  | Float a -> Printf.printf "%g" a
  | Node x -> (
    let _ = match x.v with
    | Float a -> Printf.printf "(%g," a
    | _ -> failwith "error in print_node 1"
    in
    let _ = match x.d with
    | Float a -> Printf.printf "%g)" a
    | y -> print_node_helper y; Printf.printf ")"
    in ()
    )

let print_node h n = Printf.printf "%s:" h; print_node_helper n; print_endline ""

let rec _depth x i =
  match x with
  | Node x -> _depth x.d (i + 1)
  | _ -> i

let depth x = _depth x 0

let new_node v d = { v; d; }

let unpack x =
  Array.map (fun y ->
    match y with
    | Float v -> v
    | Node x -> (
      match x.v with
      | Float v -> v
      | _ -> failwith "error"
      )
  ) x

module type MathsSig = sig
  val ( +. ) : scalar -> scalar -> scalar
  val ( *. ) : scalar -> scalar -> scalar
  val sin : scalar -> scalar
  val cos : scalar -> scalar
end

module type DerivativeSig = sig
  val add' : scalar array -> scalar array -> scalar
  val mul' : scalar array -> scalar array -> scalar
  val sin' : scalar array -> scalar array -> scalar
  val cos' : scalar array -> scalar array -> scalar
end

module rec Maths : MathsSig = struct

  let wrap_fun fn f f' args =
    let dr_mode = ref false in
    Array.iteri (fun i arg ->
      print_node (fn ^ ":" ^ (string_of_int i)) arg;
      match arg with
      | Node x -> dr_mode := true
      | _ -> ()
    ) args;
    let v = f (args |> unpack) in
    match !dr_mode with
    | true -> (
      let argsval = Array.map (fun x ->
        match x with
        | Node x -> (
          match x.d with
          | Node y -> Node (new_node x.v y.d)
          | _ -> x.v
          )
        | a -> a
      ) args
      in
      let dualval = Array.map (fun x ->
        match x with
        | Node x -> (
          match x.d with
          | Node y -> y.v
          | a -> a
          )
        | a -> Float 0.
      ) args
      in
      let d = f' dualval argsval in
      print_node "d:" d;
      Node (new_node (Float v) d)
      )
    | false -> Float v

  let ( +. ) x0 x1 = wrap_fun "add" Owl_autograd_maths.add Derivative.add' [|x0; x1|]

  let ( *. ) x0 x1 = wrap_fun "mul" Owl_autograd_maths.mul Derivative.mul' [|x0; x1|]

  let sin x = wrap_fun "sin" Owl_autograd_maths.sin Derivative.sin' [|x|]

  let cos x = wrap_fun "cos" Owl_autograd_maths.cos Derivative.cos' [|x|]

end and
Derivative : DerivativeSig = struct

  open Maths

  let add' g x = g.(0) +. g.(1)

  let mul' g x = (g.(0) *. x.(1)) +. (g.(1) *. x.(0))

  let sin' g x = g.(0) *. cos x.(0)

  let cos' g x = Float (-1.) *. g.(0) *. sin x.(0)

end


(*

open Owl_autograd;;
let n0 = {v=Float 2.; d=Float 1.};;
let n1 = {v=Float 3.; d=Node n0};;
let n2 = {v=Float 4.; d=Node n1};;
let n3 = {v=Float 5.; d=Node n2};;
Maths.sin (Node n3);;

open Owl_autograd;;
let n0 = {v=Float 1.; d=Float 1.};;
let n1 = {v=Float 1.; d=Node n0};;
let n2 = {v=Float 1.; d=Node n1};;
let n3 = {v=Float 1.; d=Node n2};;
let n4 = {v=Float 2.; d=Node n3};;
Maths.sin (Node n4);;

*)
